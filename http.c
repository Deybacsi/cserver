#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "http.h"

// server main configuration 
const int   PORT=8080,                                              // listening port
            MAXCONNS=10;                                            // max no of connections

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
    char *requestLine;
    char *requestType;
    char *key, *value, *bodyLine;

    bzero(requestString, bufferLength);                             // cleaning buffers
    bzero(responseString, bufferLength);
    if (read(request_fd, requestString, bufferLength) < 1 ) {       // try to read the request into request buffer
        fprintf(stdout, "Error while reading request, exiting.\n");
        return;
    }                  
    fprintf(stdout, "Requested:\n%s\n", requestString);

    
    
    requestLine=strtok(requestString,"\n");                         // iterating through reqeust lines
    while (requestLine != NULL)
    {
        if(strchr(requestLine, '=') != NULL)                        // to find a key=value 
        {
            bodyLine=requestLine;                                   // store it
        }
        requestLine = strtok (NULL, "\n");
    }

    requestType=strtok(requestString," ");                          // get the request type: PUT/GET from the beginning of the string

    if  (strcmp(requestType,"GET") == 0 )  {                        // GET  

        strcat(responseString, HTTP_HEADERS[0]); // HTTP 200 
    }
    else if  (strcmp(requestType,"PUT") == 0 )  {                   // PUT
        strcat(responseString, HTTP_HEADERS[0]); // HTTP 200 
    }
    else {                                                          // unknown request type
        strcat(responseString, HTTP_HEADERS[1]); // HTTP 400
        strcat(responseString, "ERROR: Call with PUT or GET requests only.\n\n");
        fprintf(stdout, "Bad request accepted, exiting.\n");
    }
    fprintf(stdout, "\nRequest:%s %s\n",requestType,bodyLine);

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
