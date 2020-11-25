#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "http.h"

// server main configuration 
const int   PORT=8080,                                              // listening port
            MAXCONNS=10,                                            // max no of connections
            BUFFERLENGTH=10000;                                     // buffer size for request/response and key value read

const char *KEYSPACEDIR="./serverdb";                               // directory path for storing key-value pairs

const char *HTTP_HEADERS[4] = {                                     // used in sendResponse()
            "HTTP/1.1 200 OK\n",
            "HTTP/1.1 400 Bad Request\n",
            "HTTP/1.1 404 Not Found\n",
            "HTTP/1.1 500 Internal Server Error\n"
};


/*
    Initialize & start the server on specified port
    Setup forking of new responder workers
*/

int startServer() {
    fprintf(stdout, "Server starting on port %i...\n", PORT);

    int listener_fd,                                                // file descriptor for listening
        request_fd;                                                 // resp fd
  
    struct sockaddr_in myAddr;                                      // to store address
    bzero( &myAddr, sizeof(myAddr));                                // clear the variable

    myAddr.sin_family = AF_INET;                                    // addressing scheme IP
    myAddr.sin_addr.s_addr = htons(INADDR_ANY);                     // connections accepted from any IP address
    myAddr.sin_port = htons(PORT);                                  // listen on port
    listener_fd = socket(AF_INET, SOCK_STREAM, 0);                  // setup listener
    bind(listener_fd, (struct sockaddr *) &myAddr, sizeof(myAddr)); // binding listener
    listen(listener_fd, MAXCONNS);                                  // start listening on port

    int forkId=-1;                                  

    while(1)
    {
        request_fd = accept(listener_fd, (struct sockaddr*) NULL, NULL);  // wait for incoming connections

        forkId=fork();                                              // if input received, try to fork a new instance
        switch (forkId) {
            case 0:                                                 // if forking was successful, this will be the new worker thread
                fprintf(stdout, "--- Worker forked successfully\n");

                checkRequest(request_fd);                           // check the request and send back a response if possible

                fprintf(stdout, "--- Worker exiting\n");
                exit(0);
                break;

            case -1:                                                // if something goes wrong print an error message
                fprintf(stdout, "Can't fork new worker!\n");
                break;

            default:                                                // the main program will continue to listen for connections
                fprintf(stdout, "Listening...\n");
                break;
        }
    }
}


void checkRequest(int request_fd) {
    char requestString[BUFFERLENGTH];                               // to store our request string
    char valueString[BUFFERLENGTH];                                 // value read from database
    char *requestLine;                                              // temp variable for request parsing
    char *requestType;                                              // request type: PUT/GET
    char *key, *value;                                              
    int keySpaceFile_fd;                                            // file descriptor for the keyspace files R/W
    char keySpaceFile_name[10000];                                  // path/filename what contains the key's value

    bzero(requestString, BUFFERLENGTH);                             // cleaning buffers
    bzero(valueString, BUFFERLENGTH);
    bzero(keySpaceFile_name, 10000);
    if (read(request_fd, requestString, BUFFERLENGTH) < 1 ) {       // try to read the request into request buffer
        fprintf(stdout, "Error while reading request, exiting.\n");
        close(request_fd);
        return;
    }                  
    // fprintf(stdout, "Requested:\n%s\n", requestString);

    key=""; value="";

    requestType=strtok(requestString," ");                          // get the first word (request type: PUT/GET) from the beginning of the string

    requestLine=strtok(NULL,"\n");                                  // iterating through reqeust lines
    while (requestLine != NULL)
    {
        if (strcmp(requestLine,"\r") == 0) {                        // strtok already stripped the \n from the \r\n  --> \r alone means it's an empty line)
            key=strtok(NULL,"=\n");                                 // parse the next line (request body) until = or \n
            value=strtok (NULL, "\n");                              // it will be (null) or given value
        }
        requestLine = strtok (NULL, "\n");
    }

    fprintf(stdout, "\nParsed request:%s %s %s\n",requestType, key, value); 

    if (key == NULL) {                                              // if there is no key in request body
        sendResponse(request_fd, 400, "ERROR: Specify a key in request body");
        fprintf(stdout, "No key specified, exiting.\n");
        close(request_fd);
        return;
    }

    char tempCh;                                                    // check for non-alphanumeric characters in keyname
    for (int i =0; i < strlen(key); i++) {
        tempCh = key[i];
         if (!isalnum(tempCh)) {                                    // if it's non AN
            sendResponse(request_fd, 400, "ERROR: Only alphanumeric characters are allowed as keyname!");
            fprintf(stdout, "Invalid keyname, exiting.\n");
            close(request_fd);
            return;
         }
    }

    strcat(keySpaceFile_name, KEYSPACEDIR); strcat(keySpaceFile_name, "/"); strcat(keySpaceFile_name, key);     // create the keyspace filename
    fprintf(stdout, "Filename: %s\n", keySpaceFile_name);

    /*
        GET request
    */
    if  (strcmp(requestType,"GET") == 0 )  {                                                                                                    

        keySpaceFile_fd=open(keySpaceFile_name, O_RDONLY);                                                      // try to open file for read
        if (keySpaceFile_fd < 0) {
            sendResponse(request_fd, 404, "ERROR: Key doesn't exist in database");
            fprintf(stdout, "Can't open file '%s', exiting.\n", keySpaceFile_name);
        } else {                                                                                                // if OK, send back its content
            read(keySpaceFile_fd, valueString, BUFFERLENGTH);
            sendResponse(request_fd, 200, valueString);
            fprintf(stdout, "Value: %s\n", valueString);
        }
        
    }
    /*
        PUT request
    */
    else if  (strcmp(requestType,"PUT") == 0 )  {                                                           
        
        if (value == NULL) {
            sendResponse(request_fd, 400, "ERROR: Specify a value for the key");
            fprintf(stdout, "No key value specified, exiting.\n");
            return;
        }
                                                          
        keySpaceFile_fd=open(keySpaceFile_name, O_WRONLY | O_TRUNC | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR );     // try to open the file for write,truncate it, if not exists then create, with RW permissions
        if (keySpaceFile_fd < 0) {                                                                              // if there are errors
            if (errno == EEXIST) {                                                                              // if the file already exists
                keySpaceFile_fd=open(keySpaceFile_name, O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR );                // open it for write, truncate the file
                if (keySpaceFile_fd < 0) {                                                                      // if it cannot be opened
                    sendResponse(request_fd, 500, "ERROR: Error while writing key file on server");
                    fprintf(stdout, "Error while writing to file '%s', exiting.\n", keySpaceFile_name);
                    return;     
                }
                write(keySpaceFile_fd, value, strlen(value));                                                   
                sendResponse(request_fd, 200, "Key modified");                                                  
            } else {                                                                                            // something else went wrong
                sendResponse(request_fd, 500, "ERROR: Error while creating key file on server");
                fprintf(stdout, "Error while creating file '%s', exiting.\n", keySpaceFile_name);
                return;
            }
        } else {                                                                                                // if the file doesn't exist -> create
            write(keySpaceFile_fd, value, strlen(value));                                                       
            sendResponse(request_fd, 200, "Key added");                                                         
        }
    }

    /*
        UNKNOWN request
    */
    else {                                                                                                  
        sendResponse(request_fd, 400, "ERROR: Call with PUT or GET requests only.");
        fprintf(stdout, "Bad request accepted, exiting.\n");
    }

    close(request_fd);
    close(keySpaceFile_fd);

}

void sendResponse(int request_fd, int statusCode, char* message) {
    char responseString[BUFFERLENGTH];                              // to store our response string
    char responseLength[50] = "";                                   // to store response length as string
    bzero(responseString, BUFFERLENGTH);

    switch (statusCode) {                                           // add HTTP status code to response header
        case 200: strcat(responseString, HTTP_HEADERS[0]); break;   
        case 400: strcat(responseString, HTTP_HEADERS[1]); break;   
        case 404: strcat(responseString, HTTP_HEADERS[2]); break;   
        case 500: strcat(responseString, HTTP_HEADERS[3]); break;   
    }       


    strcat(responseString, "Connection: close\n");                  // construct the response string 
    strcat(responseString, "Content-Length: ");                          
    sprintf(responseLength, "%d", (int) strlen(message));
    strcat(responseString, responseLength);
    strcat(responseString, "\n\n");
    strcat(responseString, message);
    write(request_fd, responseString, strlen(responseString));      // send back the whole response string

    // fprintf(stdout, "\nResponded:\n%s",responseString);
}