





#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "types.h"
#include "tcp.h"



/**/
in_addr_t inet_addr(const char *cp);


void buffer_reset(void* buff, int size){
	int i;
	char *buffer=buff;
	for(i=0; i<size; i++){
		buffer[i]='\0';
	}
}



/*
 Returns the file descriptor of the socket where it is listening
*/
int tcp_listen(){
    int sockfd;
    struct sockaddr_in servaddr;
   
    /*/ socket create and verification/*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        if(DEBUG) printf("tcp_listen > Socket create error\n");
        return -1;
    }else{
        if(DEBUG) printf("tcp_listen > Socket created\n");
    }
    buffer_reset(&servaddr, sizeof(servaddr));
   
    /* Asignar IP y PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    /*/ Binding newly created socket to given IP and verification/*/
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        if(DEBUG) printf("tcp_listen > Socket bind error\n");
        return -1;
    }else{
        if(DEBUG) printf("tcp_listen > Socket binded\n");
    }

    /*/ Now server is ready to listen and verification/*/
    if ((listen(sockfd, LISTEN_QUEUE_SIZE)) != 0) {
        if(DEBUG) printf("tcp_listen > Listen error\n");
        return -1;
    }else{
        if(DEBUG) printf("tcp_listen > Server listening\n");
    }

    return sockfd;
}



/*
 Returns the connection file descriptor of 1 client
*/
int tcp_accept(int sockfd){
    int connfd, len;
    struct sockaddr_in client;

    if(sockfd < 0){
        if(DEBUG) printf("tcp_accept > invalid sockfd = %d\n", sockfd);
        return -1;
    }

    len = sizeof(client);

    connfd = accept(sockfd, (SA*)&client, (socklen_t*) &len);
    if (connfd < 0) {
        if(DEBUG) printf("tcp_accept > server accept error\n");
        return -1;
    }
    else{
        if(DEBUG) printf("tcp_accept > server accepted client\n");
    }

    return connfd;
}




/*
 Returns socket file descriptor
 ip = "XX.XX.XX.XX"
*/
int tcp_connect(char *ip){
    int sockfd;
    struct sockaddr_in servaddr;

    if(!ip){
        if(DEBUG) printf("tcp_connect > ip = NULL\n");
        return -1;
    }
   
    /*/ socket create and verification/*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        if(DEBUG) printf("socket creation failed...\n");
        exit(0);
    }
    else{
        if(DEBUG) printf("Socket successfully created..\n");
    }
    buffer_reset(&servaddr, sizeof(servaddr));
   
    /*/ assign IP, PORT/*/
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(PORT);

    /*/ connect the client socket to server socket/*/
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        if(DEBUG) printf("connection with the server failed...\n");
        return -1;
    }
    else{
        if(DEBUG) printf("connected to the server..\n");
    }

    return sockfd;
}











