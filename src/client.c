#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.h"
#include "tcp.h"
#include "types.h"

#define MAX 80

void bzero(void* buff, int size) {
    int i;
    char* buffer = buff;
    for (i = 0; i < size; i++) {
        buffer[i] = '\0';
    }
}

void func(int sockfd) {
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit\n");
            break;
        }
        if ((strncmp(buff, "close", 5)) == 0) {
            printf("Client Close\n");
            break;
        }
    }
}

int main() {
    int sockfd, err;

    err = config_initFromFile();
    if(err){
        fprintf(stderr, "client > main > config_initFromFile error\n");
        return 1;
    }

    /*/ socket create and verification/*/
    sockfd = tcp_connect("127.0.0.1");
    if (sockfd < 0) {
        if (config_debug()) fprintf(config_debug_file(), "client main > tcp_connect error\n");
    }

    /*/ function for chat/*/
    func(sockfd);

    /*/ close the socket /*/
    close(sockfd);
    config_close_debug_file();
    return 0;
}
