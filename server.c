/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen; // 
     char buffer[256];
     char ipstr[INET_ADDRSTRLEN]; // ip sring - len of []
     struct sockaddr_in serv_addr, cli_addr; // struct of scoket addr
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     /* create socket */

     sockfd = socket(AF_INET, SOCK_STREAM, 0); // get the socket file descriptor
     if (sockfd < 0) 
        error("ERROR opening socket");

     /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     /* bind socket to this port number on this machine */

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     /* listen for incoming connection requests */

     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     /* accept a new request, create a newsockfd */

     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, ipstr, sizeof ipstr);

     printf("accepted connection from %s\n", ipstr);

     /* read message from client */
     
     while (1) {
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        if (n == 0)
         continue;
        printf("Server got %d bytes: %s", n, buffer);

        /* send reply to client */

        n = write(newsockfd,"I got your message",18);
        if (n < 0) error("ERROR writing to socket");
        
        if (strncmp(buffer, "exit", 4) == 0) {
                printf("server exiting\n");
                break;
        }
     }
     return 0; 
}
