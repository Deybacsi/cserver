#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "http.h"

// server main configuration 
const int   PORT=8080,                                              // listening port
            MAXCONNS=10,                                            // max no of connections
            BUFFERLENGTH=10000;                                   // request/response buffer size  

const char *KEYSPACEDIR="./serverdb";                               // directory path for storing key-value pairs

const char *HTTP_HEADERS[3] = {
            "HTTP/1.1 200 OK\n",
            "HTTP/1.1 400 Bad Request\n",
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
                fprintf(stdout, "Worker forked successfully\n");

                checkRequest(request_fd);                           // send back a response

                fprintf(stdout, "Worker exiting\n");
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
    char *requestLine;                                              // temp variable for request parsing
    char *requestType;                                              // request type: PUT/GET
    char *bodyLine;                                                 // the key=value string from request
    char *key, *value;                                              //      splitted into 2 vars
    int keySpaceFile_fd;                                            // file descriptor for the keyspace files
    char keySpaceFile_name[10000];                                  // path/filename that contains the key's value

    bzero(requestString, BUFFERLENGTH);                             // cleaning buffers
    
    bzero(keySpaceFile_name, 10000);
    if (read(request_fd, requestString, BUFFERLENGTH) < 1 ) {       // try to read the request into request buffer
        fprintf(stdout, "Error while reading request, exiting.\n");
        return;
    }                  
    fprintf(stdout, "Requested:\n%s\n", requestString);

    bodyLine=""; key=""; value="";
    
    requestLine=strtok(requestString,"\n");                         // iterating through reqeust lines
    while (requestLine != NULL)
    {
        if(strchr(requestLine, '=') != NULL)                        // to find a key=value 
        {
            bodyLine=requestLine;                                   // store this line for further use
        }
        requestLine = strtok (NULL, "\n");
    }

    requestType=strtok(requestString," ");                          // get the first word (request type: PUT/GET) from the beginning of the string
    key=strtok(bodyLine,"=");                                       // get key from request body
    value=strtok (NULL, "\n");                                      // and value

    strcat(keySpaceFile_name, KEYSPACEDIR);
    strcat(keySpaceFile_name, "/");
    strcat(keySpaceFile_name, key);
    
    fprintf(stdout, "Filename:\n%s\n", keySpaceFile_name);

    if  (strcmp(requestType,"GET") == 0 )  {                        // GET  

        
    }

    else if  (strcmp(requestType,"PUT") == 0 )  {                                                           // PUT
        keySpaceFile_fd=open(keySpaceFile_name, O_WRONLY | O_TRUNC | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR );     // try to open the file for write,truncate it, if not exists then create, with RW permissions
        if (keySpaceFile_fd < 0) {                                                                              // if there are errors
            if (errno == EEXIST) {                                                                          // if the file already exists
                keySpaceFile_fd=open(keySpaceFile_name, O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR );                // open it for write, truncate the file
                write(keySpaceFile_fd, value, strlen(value));                                                   // write value
                sendResponse(request_fd, 200, "SUCCESS: Key modified");                                         // send back http 200 & response
            } else {                                                                                        // something else went wrong
                sendResponse(request_fd, 500, "ERROR: Error while creating key file on server");
                fprintf(stdout, "Error while creating file '%s', exiting.\n", keySpaceFile_name);
                return;
            }
        } else {                                                                                            // if the file doesn't exist -> create
            write(keySpaceFile_fd, value, strlen(value));                                                       // write the value into it
            sendResponse(request_fd, 200, "SUCCESS: Key added");                                                // send back http 200 & response
        }
    }

    else {                                                          // unknown request type
        sendResponse(request_fd, 400, "ERROR: Call with PUT or GET requests only.");
        fprintf(stdout, "Bad request accepted, exiting.\n");
    }

    fprintf(stdout, "\nParsed request:%s %s %s\n",requestType, key, value);
}

void sendResponse(int request_fd, int statusCode, char* message) {
    char responseString[BUFFERLENGTH];                              // to store our response string
    char responseLength[50] = "";                                   // to store response length as string
    bzero(responseString, BUFFERLENGTH);

    switch (statusCode) {
        case 200: strcat(responseString, HTTP_HEADERS[0]); break;   // HTTP 200 
        case 400: strcat(responseString, HTTP_HEADERS[1]); break;   // HTTP 400
        case 500: strcat(responseString, HTTP_HEADERS[2]); break;   // HTTP 500
    }       

    strcat(responseString, "Content-Length: ");                     // construct the response string      
    sprintf(responseLength, "%d", (int) strlen(message));
    strcat(responseString, responseLength);
    strcat(responseString, "\n");
    strcat(responseString, message);
    strcat(responseString, "\n");
    strcat(responseString, "\n");
    write(request_fd, responseString, strlen(responseString)+1);    // send back the response

    fprintf(stdout, "\nResponded:\n%s",responseString);
}