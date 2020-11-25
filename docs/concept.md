## C language

One of the main purposes of this project is to demonstrare C language, its possibilities, and its usage.

Because of this, the project was separated to multiple source code files to demonstrate:
- the use of header files
- the use of gcc compiler
- the use of make and Makefiles

The number of files was keeped as minimal as possible.

There is a `compile.sh` script to easily compile and run the server

## Configurability

The server's main properties should be (somewhat) easily reconfigured in the future.

The below properties can be freely modified directly in `http.c`
 
```
// server main configuration 
const int   PORT=8080,                                              // listening port
            MAXCONNS=10,                                            // max no of connections
            BUFFERLENGTH=10000;                                     // buffer size for request/response and key value read

const char *KEYSPACEDIR="./serverdb";                               // directory path for storing key-value pairs
```

## Concurrent connections

The server uses the fork() function to duplicate itself on an incoming request, and the main instance continues with listening.

The forked copy (called as worker) will continue it's running with checkRequest(), and hopefully with the sendResponse() function if no fatal errors occured. (For eg. client unexpectedly closed connection)

## Parsing HTTP requests - checkRequest()

Incoming HTTP request are in plain text, strtok() was used to split the input string, and parse the GET/PUT request type, and the key=value from the request body.

## Storing / getting the data - checkRequest()

The key storage is accessible (by default) in the `serverdb` directory.
As the project's specification states, key names can contain only alphanumerical characters, so the easiest and fastest storing method was implemented: 

- keys are stored as individual files
- file name is key name
- file content is key value

(This method should not be used on NTFS or FAT filesystems, it can cause heavy slowdowns)

PUT requests always overwrite existing key values.

## Sending response - sendResponse()

It composes the final response string (HTTP headers + body) and sends it back to the client.

## Error handling

The code tries to handle the following issues:

- the main program can't fork a new worker
- client disconnect unintentionally while reading request
- no key is given in request body
- key name contains non-alphanumerical characters
- GET requests

    - key doesn't exists in the storaga
- PUT requests

    - no value was specified for storing 
    - try to avoid concurrent creation of a key file by multiple clients by using open() with O_CREAT | O_EXCL
    - try to avoid concurrent writes to the same existing file by multiple clients

- other requests than GET/PUT

Except of the first 2 cases, every error both sends back a response message to the client, and a message to stdout, as a possibility for server-side logging.





