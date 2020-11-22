#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "http.h"

// server main configuration 
const int   PORT=8080,                              // listening port
            MAXCONNS=10;                            // max no of connections

/*
    Initialize & start the server on specified port
    Setup forking of new responder workers
*/

int startServer() {
    fprintf(stdout, "Server starting on port %i...\n", PORT);

    int listener,       // file descriptor for listening
        request;        // resp fd

    
    struct sockaddr_in myAddr;                      // to store address
    bzero( &myAddr, sizeof(myAddr));                // clear the variable

    myAddr.sin_family = AF_INET;                    // addressing scheme IP
    myAddr.sin_addr.s_addr = htons(INADDR_ANY);     // connections accepted from any IP address
    myAddr.sin_port = htons(PORT);                  // listen on port
    listener = socket(AF_INET, SOCK_STREAM, 0);     // setup listener
    bind(listener, (struct sockaddr *) &myAddr, sizeof(myAddr));    // binding listener
    listen(listener, MAXCONNS);                     // start listening on port

    
    char str[100];

    int forkId=-1;

    while(1)
    {
        request = accept(listener, (struct sockaddr*) NULL, NULL);  // wait for incoming connections

        forkId=fork();                                              // if input received, try to fork a new instance
        switch (forkId) {
            case 0:                                                 // if forking was successful, this will be the new worker thread
                fprintf(stdout, "Worker forked successfully\n");
                
                bzero( str, 100);
                read(request,str,100);
                fprintf(stdout, "Echo:\n%s", str);
                write(request, str, strlen(str)+1);

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

 
