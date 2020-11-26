
// starts the server on the specified port (in http.c)
int startServer();

// called, when a request is incoming
// checks for errors, and tries to send back response with sendResponse()
void checkRequest(int request_fd);

// sends back a response with HTTP status code, and a custom message string as body
void sendResponse(int request_fd, int statusCode, char* message);
