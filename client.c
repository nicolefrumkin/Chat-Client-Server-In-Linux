#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, last = 0; 
    // socketfd - socket file descriptor

    struct sockaddr_in serv_addr; 
    /* creates a variable to store the server's parameters:
    sin_family: The address family (e.g., IPv4).
    sin_port: The port number of the server.
    sin_addr: The IP address of the server. */
    struct hostent *server; 
    /* creates a pointer named server to hold information about the server's hostname, 
    including its IP address, which will be retrieved later using the gethostbyname() function. */

    /* server helps resolve the server's hostname into an IP address using gethostbyname().
    serv_addr stores the resolved IP address and port number in the format required by networking functions like connect().*/
    
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    /* create socket, get sockfd handle */

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /* AF_INET specifies the IPv4 address family,
    SOCK_STREAM sets the socket type to a reliable, connection-based protocol (TCP),
    and 0 lets the system choose the default protocol for TCP.*/
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    /* fill in server address in sockaddr_in datastructure */

    server = gethostbyname(argv[1]); 
    // looks up in the DNS resolver the hostname provided and stores its information, including the IP address
    // returns a struct hostent with a list of IP addresses (h_addr_list) that match the given hostname.
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    //sets to zero the memory of the serv_addr structure
    // bcopy is an old C function that copies memory from one place to another, commonly used in older Unix systems.
    serv_addr.sin_family = AF_INET; // sets the address family of the serv_addr structure to AF_INET, which means it will use the IPv4 protocol.
    //bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    /* copies the server's IP address from the first address in the resolved list
    into sin_addr where sin_addr is a field in the struct sockaddr_in that represents the IP address of the server */
    serv_addr.sin_port = htons(portno);
    // sets the server's port number in the serv_addr structure, converting it from 16 bit integer to the network byte order using htons (Host TO Network Short).
    
    /* connect to server */

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    else{
        write(sockfd,argv[3],strlen(argv[3]));
    }
    /* ask user for input */

    while (1) {
            printf("Please enter the message: ");
            bzero(buffer,256);
            fgets(buffer,255,stdin);

            /* send user message to server */

            n = write(sockfd,buffer,strlen(buffer));
            if (n < 0) 
                 error("ERROR writing to socket");
            // check if client wants to exit
            if (strncmp(buffer, "exit", 4) == 0) {
                last = 1;
                printf("client exiting\n");
                break;
            }

            bzero(buffer,256);
            /* read reply from server */

            n = read(sockfd,buffer,255);
            if (n < 0) 
                 error("ERROR reading from socket");
            printf("server replied %d bytes: %s\n",n, buffer);

            if (last){
                printf("client exiting\n");
                break;
            }
            
    }

    return 0;
}
