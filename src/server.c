
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> /*Para inet_ntop*/
#include "tcp.h"
#include "types.h"
#include "process.h"
#define MAX 8000
#define PORT 8080
#define SA struct sockaddr


/*/ Driver function/*/
int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr;
    char ip[50];
   
    sockfd = tcp_listen();
    if(sockfd < 0){
        if(DEBUG) printf("server main > tcp_listen error\n");
        return 1;
    }


    inet_ntop(AF_INET, &(servaddr.sin_addr), ip, INET_ADDRSTRLEN);
    printf("Server IP = %s\n", ip);

    while(TRUE){

        /*/ Accept the data packet from client and verification/*/
        connfd = tcp_accept(sockfd);
        if(connfd < 0){
            if(DEBUG) printf("server main > tcp_accept error\n");
            
            close(sockfd);
            return 2;
        }

	    if(DEBUG) {
            printf("##############################\n");
            printf("//                          //\n");
            printf("//  Peticion -> Connfd = %d //\n", connfd);
            printf("//                          //\n");
            printf("##############################\n");
        }

        process_request(connfd);

    }
   
    /*/ After chatting close the socket/*/
    close(sockfd);
	return 0;
}
