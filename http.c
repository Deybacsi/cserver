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
            MAXCONNS=10;                                            // max no of connections

const char *KEYSPACEDIR="./serverdb";                               // directory path for storing key-value pairs

const char *HTTP_HEADERS[2] = {
            "HTTP/1.1 200 OK\n",
            "HTTP/1.1 400 Bad Request\n"
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

                sendResponse(request_fd);                           // send back a response

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


void sendResponse(int request_fd) {
    const int bufferLength=10000;                                   // request/response buffer size      
    char requestString[bufferLength];                               // to store our request string
    char responseString[bufferLength];                              // to store our response string
    char responseLength[50] = "";                                   // to store response length in string
    char *requestLine;                                              // temp variable for request parsing
    char *requestType;                                              // request type: PUT/GET
    char *bodyLine;                                                 // the key=value string from request
    char *key, *value;                                              //      splitted into 2 vars
    int keySpaceFile_fd;                                            // file descriptor for the keyspace files
    char keySpaceFile_name[10000];                                  // path/filename that contains the key's value

    bzero(requestString, bufferLength);                             // cleaning buffers
    bzero(responseString, bufferLength);
    bzero(keySpaceFile_name, 10000);
    if (read(request_fd, requestString, bufferLength) < 1 ) {       // try to read the request into request buffer
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
    
    fprintf(stdout, "aaaaaaaaaaaaa:\n%s\n", keySpaceFile_name);

    if  (strcmp(requestType,"GET") == 0 )  {                        // GET  

        strcat(responseString, HTTP_HEADERS[0]); // HTTP 200 
    }
    else if  (strcmp(requestType,"PUT") == 0 )  {                   // PUT
        keySpaceFile_fd=open(keySpaceFile_name, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR );
        if (keySpaceFile_fd < 0) {                                  // if there are errors
            if (errno == EEXIST) {                                  // if the file already exists

            } else {                                                // something else went wrong
                fprintf(stdout, "Error while creating file '%s', exiting.\n", keySpaceFile_name);
                return;
            }
        } else {                                                    // if the file don't exists
            write(keySpaceFile_fd, value, strlen(value));
        }
        strcat(responseString, HTTP_HEADERS[0]); // HTTP 200 
    }
    else {                                                          // unknown request type
        strcat(responseString, HTTP_HEADERS[1]); // HTTP 400
        strcat(responseString, "ERROR: Call with PUT or GET requests only.\n\n");
        fprintf(stdout, "Bad request accepted, exiting.\n");
    }
    fprintf(stdout, "\nParsed request:%s %s %s\n",requestType, key, value);

    char a[]="some response string";
    strcat(responseString, "Content-Length: ");                     // construct the response string      
    sprintf(responseLength, "%d", (int) strlen(a));
    strcat(responseString, responseLength);
    strcat(responseString, "\n");
    strcat(responseString, a);
    strcat(responseString, "\n");
    strcat(responseString, "\n");

    write(request_fd, responseString, strlen(responseString)+1);    // send back the response

    fprintf(stdout, "\nResponded:\n%s",responseString);

}
