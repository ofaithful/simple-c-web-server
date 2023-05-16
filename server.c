#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <unistd.h>

#pragma comment(lib, "ws2_32.lib")

#include "lib/response.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define HOST "127.0.0.1"

void handleHtml(int clientSocket) {
    FILE* file = fopen("index.html", "r");
    if (file == NULL) {
        perror("failed to open a file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    printf("size of the index.html is: %d\n", size);
    fseek(file, 0, SEEK_SET);

    char response[size];


    size_t bytesRead;
    size_t totalBytesRead = 0;
    while ((bytesRead = fread(response, 1, size, file)) > 0) {
        totalBytesRead += bytesRead;
    }

    fclose(file);

    char headers[BUFFER_SIZE];

    sprintf(
        headers,
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %lu\r\n"
        "Content-Type: text/html\r\n"
        "\r\n",
        totalBytesRead
    );

    printf("headers: %s", headers);

    if (send(clientSocket, headers, strlen(headers), 0) == -1) {
        perror("failed to send headers");
        exit(EXIT_FAILURE);
    }

    if (send(clientSocket, response, totalBytesRead, 0) == -1) {
        perror("failed to send response");
        exit(EXIT_FAILURE);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientAddressLength = sizeof(clientAddress);
    
    char buffer[BUFFER_SIZE] = {0};

    // initialize winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }

    // create a socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        return 1;
    }

    // bind the socket to a specific IP address and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(HOST);
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        perror("bind failed");
        return 1;
    }

    // listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        perror("listen failed");
        return 1;
    }
    printf("Web server started on %s:%d\n", HOST, PORT);

    // accept incoming connections and handle them
    while (1) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength)) < 0) {
            perror("accept failed");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);

        printf("[%s:%u] %s %s %s\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), method, version, uri);
        printf("Received request: \n%s\n", buffer);

        if (*uri == '/') {
            handleHtml(clientSocket);
        } else {
            char *response = getResponse(method);
            send(clientSocket, response, strlen(response), 0);
            printf("Response sent:\n%s\n", response);
            closesocket(clientSocket);
        }
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}