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
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_CLIENTS 16
#define MAX_MSG 256

// Define a struct to represent a client
typedef struct client {
    int socket_fd;               // Socket file descriptor for communication
    struct sockaddr_in address;  // Client address (IP and port)
    char name[50];               // Client's name or identifier
    char ipstr[INET_ADDRSTRLEN]; // Human-readable IP
    int is_active;               // Flag to check if the client is active
} client;
