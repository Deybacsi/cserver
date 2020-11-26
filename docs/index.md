The purpose of this project is to implement a simple key-value storage server in C, and a very basic Python client to check its functionalities.

## OS

Server and client both runs on Linux. (Tested on Ubuntu 18.04)

## The server

- Implemented in C
- Contains a Makefile
- Doesn't use any external libraries
- Stores key-value pairs
    
    - Key and value are ASCII strings
    - Key can contain just numbers and letters

- Responds to the following HTTP requests

    - PUT key value : stores a key in the database with the given value
    - GET key : retrieve a previously stored value for key

- Is able to serve multiple clients
- Uses shared key-space between clients

## The client

- Implemented in Python
- Provides a basic command line interface to PUT/GET keys
