/* A simple server in the internet domain using TCP */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> // For fcntl
#include <errno.h> // For errno
#define MAX_CLIENTS 16

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Define a struct to represent a client
typedef struct client {
    int socket_fd;               // Socket file descriptor for communication
    struct sockaddr_in address;  // Client address (IP and port)
    char name[50];               // Client's name or identifier
    char ipstr[INET_ADDRSTRLEN]; // Human-readable IP
    int is_active;               // Flag to check if the client is active
} client;

// Function to set a socket to non-blocking mode
void set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        error("fcntl(F_GETFL) failed");
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        error("fcntl(F_SETFL) failed");
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    client *clients[MAX_CLIENTS] = {NULL};
    int n;

    // Initialize client slots
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = malloc(sizeof(client));
        clients[i]->is_active = 0;
        clients[i]->socket_fd = -1;
    }

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // Set server socket to non-blocking
    set_non_blocking(sockfd);

    // Fill in port number and address
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind socket to port
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    // Listen for incoming connections
    listen(sockfd, 5);

    printf("Server started on port %d\n", portno);

    fd_set read_fds, write_fds;
    int max_fd = sockfd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        // Add server socket to read set
        FD_SET(sockfd, &read_fds);

        // Add active clients to sets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i]->is_active) {
                FD_SET(clients[i]->socket_fd, &read_fds);
                FD_SET(clients[i]->socket_fd, &write_fds);
                if (clients[i]->socket_fd > max_fd) {
                    max_fd = clients[i]->socket_fd;
                }
            }
        }

        // Use select to monitor sockets
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            error("select failed");
        }

        // Check for new connections
        if (FD_ISSET(sockfd, &read_fds)) {
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("ERROR on accept");
                }
            } else {
                set_non_blocking(newsockfd);
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i]->is_active == 0) {
                        clients[i]->is_active = 1;
                        clients[i]->socket_fd = newsockfd;
                        inet_ntop(AF_INET, &cli_addr.sin_addr, clients[i]->ipstr, INET_ADDRSTRLEN);
                        printf("New client connected from %s\n", clients[i]->ipstr);
                        break;
                    }
                }
            }
        }

        // Handle client communication
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i]->is_active && FD_ISSET(clients[i]->socket_fd, &read_fds)) {
                bzero(buffer, 256);
                n = recv(clients[i]->socket_fd, buffer, 255, 0);
                if (n <= 0) {
                    if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                        printf("Client %s disconnected\n", clients[i]->ipstr);
                        close(clients[i]->socket_fd);
                        clients[i]->is_active = 0;
                    }
                } else {
                    if(buffer[strlen(buffer)-1]=='\n'){
                        printf("Message from %s: %s", clients[i]->ipstr, buffer);
                    } 
                    else{
                        printf("Message from %s: %s\n", clients[i]->ipstr, buffer);
                    }
                    send(clients[i]->socket_fd, "Message received", 17, 0);
                }
            }
        }
    }

    // Cleanup
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            if (clients[i]->is_active) {
                close(clients[i]->socket_fd);
            }
            free(clients[i]);
        }
    }

    close(sockfd);
    return 0;
}
