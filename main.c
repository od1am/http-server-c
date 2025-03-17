#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define port 8080
#define BUFFER_SIZE 1024

void handle_hello(int client_socket) {
    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!\n";
    send(client_socket, response, strlen(response), 0);
}

void handle_headers(int client_socket, char *request) {
    char response[BUFFER_SIZE];
    char *header_start = strstr(request, "\r\n") + 2;
    char *header_end = strstr(header_start, "\r\n\r\n");

    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
    send(client_socket, response, strlen(response), 0);

    char *header = strtok(header_start, "\r\n");
    while (header != NULL && header < header_end) {
        snprintf(response, sizeof(response), "%s\n", header);
        send(client_socket, response, strlen(response), 0);
        header = strtok(NULL, "\r\n");
    }
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    recv(client_socket, buffer, sizeof(buffer), 0);

    if (strncmp(buffer, "GET /hello", 10) == 0) {
        handle_hello(client_socket);
    } else if (strncmp(buffer, "GET /headers", 12) == 0) {
        handle_headers(client_socket, buffer);
    } else {
        char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found\n";
        send(client_socket, response, strlen(response), 0);
    }
    close(client_socket);
}

int main () {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_request(client_socket);
    }

    close(server_socket);
    return 0;
}