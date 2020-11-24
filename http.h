

int startServer();
void checkRequest(int request_fd);
void sendResponse(int request_fd, int statusCode, char* message);
