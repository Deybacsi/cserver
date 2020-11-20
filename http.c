#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#include "http.h"

const int   PORT=8080;
const int   MAXCONNS=10;


int startServer() {
    fprintf(stdout, "Server starting on port %i...\n",PORT);

    int listener,   // file descriptor for listening
        req;       // resp fd

    // to store address
    struct sockaddr_in myAddr;
    // clear the variable
    bzero( &myAddr, sizeof(myAddr));

    // addressing scheme IP
    myAddr.sin_family = AF_INET;
    // connect from any IP
    myAddr.sin_addr.s_addr = htons(INADDR_ANY);
    // listen on port
    myAddr.sin_port = htons(PORT);

    // setup listener
    listener = socket(AF_INET, SOCK_STREAM, 0);
    // bindign listener
    bind(listener, (struct sockaddr *) &myAddr, sizeof(myAddr));
    // start listening on port
    listen(listener, MAXCONNS);

    req = accept(listener, (struct sockaddr*) NULL, NULL);

    char str[100];

    while(1)
    {
 
        bzero( str, 100);
 
        read(req,str,100);
 
        printf("Echoing back - %s",str);
 
        write(req, str, strlen(str)+1);
 
    }
}