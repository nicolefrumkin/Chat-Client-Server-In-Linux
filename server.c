#include "hw3.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}



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
    char buffer[MAX_MSG];
    struct sockaddr_in serv_addr, cli_addr;
    client *clients[MAX_CLIENTS] = {NULL};
    int n;
    int opt = 1; // Option value for SO_REUSEADDR
    int connected_users = 0;

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

    // Set socket options to reuse address
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error("ERROR setting SO_REUSEADDR");
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
                if(connected_users < MAX_CLIENTS){
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        if (clients[i]->is_active == 0 ) {
                            clients[i]->is_active = 1;
                            connected_users++;
                            send(newsockfd,"y", 1, 0); // signaling for a working connection
                            clients[i]->socket_fd = newsockfd;
                            inet_ntop(AF_INET, &cli_addr.sin_addr, clients[i]->ipstr, INET_ADDRSTRLEN);

                            // Receive the client name BEFORE setting non-blocking mode
                            bzero(buffer, MAX_MSG);
                            n = recv(newsockfd, buffer, MAX_MSG - 1, 0);
                            if (n > 0) {
                                buffer[n] = '\0';
                                strncpy(clients[i]->name, buffer, sizeof(clients[i]->name) - 1);
                                clients[i]->name[sizeof(clients[i]->name) - 1] = '\0';
                                printf("Client %s connected from %s\n", clients[i]->name, clients[i]->ipstr);
                            } else {
                                strncpy(clients[i]->name, "Unknown", sizeof(clients[i]->name) - 1);
                                clients[i]->name[sizeof(clients[i]->name) - 1] = '\0';
                                printf("Client connected from %s with no name provided\n", clients[i]->ipstr);
                            }

                            // Set non-blocking mode AFTER receiving the name
                            set_non_blocking(newsockfd);
                            break;
                        }
                    }
                }
                else{
                    send(newsockfd,"n", 1, 0); // cannot connect
                    close(newsockfd);
                }
            }

        }

        // Handle client communication
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i]->is_active && FD_ISSET(clients[i]->socket_fd, &read_fds)) {
                char sender[MAX_MSG];
                strcpy(sender,clients[i]->name);
                bzero(buffer, MAX_MSG);
                n = recv(clients[i]->socket_fd, buffer, 255, 0);
                if (n <= 0) { // debug later - what to do in case of no msg
                    if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                        printf("Client %s disconnected\n", clients[i]->name);
                        close(clients[i]->socket_fd);
                        clients[i]->is_active = 0;
                        connected_users--;
                    }
                } else {
                    if (buffer[0] == '@') { // whisper
                        char target_buffer[MAX_MSG];
                        strncpy(target_buffer, buffer + 1, MAX_MSG - 1); // Skip the '@' and copy the rest
                        target_buffer[MAX_MSG - 1] = '\0'; // Ensure null termination

                        // Extract the target name and message
                        char* target_name = strtok(target_buffer, " "); // First token is the target name
                        char* message = strtok(NULL, ""); // Remaining string is the message

                        if (target_name != NULL && message != NULL) {
                            for (int i = 0; i < MAX_CLIENTS; i++) {
                                if (clients[i]->is_active && strcmp(clients[i]->name, target_name) == 0) { // Match client name
                                    char send_buffer[2 * MAX_MSG];
                                    snprintf(send_buffer, sizeof(send_buffer), "%s: %s", sender, message);
                                    if(send_buffer[strlen(send_buffer)-1]=='\n'){
                                        send_buffer[strlen(send_buffer)-1]='\0';
                                    } 
                                    send(clients[i]->socket_fd, send_buffer, strlen(send_buffer), 0); 
                                    break; // Stop after finding the target
                                }
                            }
                        }
                    }
                    else { // broadcast
                        if(buffer[strlen(buffer)-1]=='\n'){
                            printf("%s: %s", clients[i]->name, buffer);
                            buffer[strlen(buffer)-1]='\0';
                        } 
                        else{
                            printf("%s: %s\n", clients[i]->name, buffer);
                        }
                        // Send the message to all other active clients
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            char send_buffer[2*MAX_MSG];
                            snprintf(send_buffer, sizeof(send_buffer), "%s: %s", clients[i]->name, buffer);
                            send(clients[j]->socket_fd, send_buffer, strlen(send_buffer), 0); 
                        }
                    }
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
