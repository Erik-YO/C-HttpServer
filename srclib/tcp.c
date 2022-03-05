

#include "tcp.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "types.h"

/**/
in_addr_t inet_addr(const char *cp);

void buffer_reset(void *buff, int size) {
    int i;
    char *buffer = buff;
    for (i = 0; i < size; i++) {
        buffer[i] = '\0';
    }
}

/*
 Returns the file descriptor of the socket where it is listening
*/
int tcp_listen() {
    int sockfd;
    struct sockaddr_in servaddr;
    int flag = 1;

    /*/ socket create and verification/*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        if (config_debug()) fprintf(config_debug_file(), "tcp_listen > Socket create error\n");
        return -1;
    } else {
        if (config_debug()) fprintf(config_debug_file(), "tcp_listen > Socket created\n");
    }
    buffer_reset(&servaddr, sizeof(servaddr));

    /* Asignar IP y PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    /* Opciones Reusable */
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
        if (config_debug()) fprintf(config_debug_file(), "tcp_listen > Socket options error\n");
        return -2;
    }

    /*/ Binding newly created socket to given IP and verification/*/
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
        if (config_debug()) fprintf(config_debug_file(), "tcp_listen > Socket bind error\n");
        return -1;
    } else {
        if (config_debug()) fprintf(config_debug_file(), "tcp_listen > Socket binded\n");
    }

    /*/ Now server is ready to listen and verification/*/
    if ((listen(sockfd, LISTEN_QUEUE_SIZE)) != 0) {
        if (config_debug()) fprintf(config_debug_file(), "tcp_listen > Listen error\n");
        return -1;
    } else {
        if (config_debug()) fprintf(config_debug_file(), "tcp_listen > Server listening\n");
    }

    return sockfd;
}

/*
 Returns the connection file descriptor of 1 client
*/
int tcp_accept(int sockfd) {
    int connfd, len;
    struct sockaddr_in client;

    if (sockfd < 0) {
        if (config_debug()) fprintf(config_debug_file(), "tcp_accept > invalid sockfd = %d\n", sockfd);
        return -1;
    }

    len = sizeof(client);

    connfd = accept(sockfd, (SA *)&client, (socklen_t *)&len);
    if (connfd < 0) {
        if (config_debug()) fprintf(config_debug_file(), "tcp_accept > server accept error\n");
        return -1;
    } else {
        if (config_debug()) fprintf(config_debug_file(), "tcp_accept > server accepted client\n");
    }

    return connfd;
}

/*
 Returns socket file descriptor
 ip = "XX.XX.XX.XX"
*/
int tcp_connect(char *ip) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (!ip) {
        if (config_debug()) fprintf(config_debug_file(), "tcp_connect > ip = NULL\n");
        return -1;
    }

    /*/ socket create and verification/*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        if (config_debug()) fprintf(config_debug_file(), "socket creation failed...\n");
        exit(0);
    } else {
        if (config_debug()) fprintf(config_debug_file(), "Socket successfully created..\n");
    }
    buffer_reset(&servaddr, sizeof(servaddr));

    /*/ assign IP, PORT/*/
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(PORT);

    /*/ connect the client socket to server socket/*/
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
        if (config_debug()) fprintf(config_debug_file(), "connection with the server failed...\n");
        return -1;
    } else {
        if (config_debug()) fprintf(config_debug_file(), "connected to the server..\n");
    }

    return sockfd;
}
