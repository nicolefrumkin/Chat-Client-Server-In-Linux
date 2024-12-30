#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>

void error(char *msg) {
    perror(msg);
    exit(0);
}

// Function to handle SIGCHLD for cleaning up child processes
void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    pid_t pid;
    char test;

    // Set up SIGCHLD handler to avoid zombie processes
    signal(SIGCHLD, handle_sigchld);

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    // Create socket
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // Send the client name to the server
    n = write(sockfd, argv[3], strlen(argv[3]));
    if (n < 0)
        error("ERROR sending name to server");

    n = read(sockfd, &test, 1);
    if(test == 'y'){
        printf("Connected to the server as %s.\n", argv[3]);
    }
    else if(test == 'n'){
        printf("server if full, cannot connect\n");
        return 0;
    }

    // Fork to handle receiving and sending
    pid = fork();

    if (pid < 0) {
        error("ERROR forking");
    } else if (pid == 0) {
        // Child process: handles receiving messages
        while (1) { 
            bzero(buffer, 256);
            n = read(sockfd, buffer, 255);
            if (n <= 0) {
                if (n == 0) {
                    printf("Server disconnected.\n");
                } else {
                    perror("ERROR reading from socket");
                }
                close(sockfd);
                exit(0);
            }
            printf("%s\n", buffer);
        }
    } else{
        // Parent process: handles sending messages
        while (1) {
            usleep(1000); // waiting for child to write back
            printf("Please enter the message: \n");
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);
            // Send message to the server
            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0){
                error("ERROR writing to socket");
                continue;
            }
            // Check if the client wants to exit
            if (strncmp(buffer, "!exit", 5) == 0) {
                printf("Client exiting\n");
                kill(pid, SIGTERM); // Kill the child process
                break;
            }
        }
        // Wait for the child process to finish
        wait(NULL);
        close(sockfd);
    }

    return 0;
}
    