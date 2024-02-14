#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the HTTP request from the client
    bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        perror("Failed to read from client");
        close(client_socket);
        return;
    }

    // Null-terminate the buffer to treat it as a string
    buffer[bytes_read] = '\0';

    // Parse the HTTP request
    char *filename = strtok(buffer, " ");
    if (strcmp(filename, "GET") != 0) {
        close(client_socket);
        return;
    }
    filename = strtok(NULL, " ");

    // Open the requested file
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s%s", getenv("HOME"), filename);
    int file = open(full_path, O_RDONLY);
    if (file == -1) {
        perror("Failed to open file");
        close(client_socket);
        return;
    }

    // Send HTTP headers
    char headers[] = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n";
    write(client_socket, headers, strlen(headers));

    // Send file contents
    ssize_t bytes_sent;
    while ((bytes_read = read(file, buffer, BUFFER_SIZE)) > 0) {
        bytes_sent = write(client_socket, buffer, bytes_read);
        if (bytes_sent != bytes_read) {
            perror("Failed to send file");
            close(client_socket);
            close(file);
            return;
        }
    }

    // Close the file and the client socket
    close(file);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 5) == -1) {
        perror("Failed to listen for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept and handle connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Failed to accept connection");
            continue;
        }

        // Handle client request in a new process
        if (fork() == 0) {
            close(server_socket);
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}
