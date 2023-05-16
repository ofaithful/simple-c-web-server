#include <string.h>
#include "response.h"

char* getResponse(char *method) {
    if (strcmp(method, "GET") == 0) {
        return "HTTP/1.1 200 OK\r\n"
            "Server: server.exe\r\n"
            "Content-type: application/json\r\n\r\n"
            "{ \"method\": \"GET\" }\r\n";
    } else if (strcmp(method, "POST") == 0) {
        return "HTTP/1.1 200 OK\r\n"
            "Server: server.exe\r\n"
            "Content-type: application/json\r\n\r\n"
            "{ \"method\": \"POST\" }\r\n";
    } else if (strcmp(method, "PATCH") == 0) {
        return "HTTP/1.1 200 OK\r\n"
            "Server: server.exe\r\n"
            "Content-type: application/json\r\n\r\n"
            "{ \"method\": \"PATCH\" }\r\n";
    } else if (strcmp(method, "DELETE") == 0) {
        return "HTTP/1.1 200 OK\r\n"
            "Server: server.exe\r\n"
            "Content-type: application/json\r\n\r\n"
            "{ \"method\": \"DELETE\" }\r\n";
    } else {
        return "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-type: plain/text\r\n"
            "Allow: GET, POST, PATCH, DELETE\r\n\r\n"
            "405 Try another method\r\n";
    }
}