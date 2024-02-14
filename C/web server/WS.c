#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define MAX_SIZE 1024

void serve_client(int client_socket) {
    char buffer[MAX_SIZE] = {0};
    FILE *html_file;

    html_file = fopen("public/index.html", "r");
    if (html_file == NULL) {
        fprintf(stderr, "Error opening HTML file\n");
        return;
    }

    sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    send(client_socket, buffer, strlen(buffer), 0);

    while (fgets(buffer, sizeof(buffer), html_file) != NULL) {
        send(client_socket, buffer, strlen(buffer), 0);
    }

    fclose(html_file);
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        serve_client(client_socket);
    }

    return 0;
}