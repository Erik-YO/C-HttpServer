
#include <arpa/inet.h> /*Para inet_ntop*/
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "process.h"
#include "tcp.h"
#include "types.h"
#define MAX 8000
#define PORT 8080
#define SA struct sockaddr

#define HELP_TEXT \
    "\
\nHelp:\n\
\tCtrl + C to stop the server\n\
\n\
"

/*/ Driver function/*/
int main(int argc, char const *argv[]) {
    int sockfd, connfd;
    struct sockaddr_in servaddr;
    char ip[50];

    process_setSignalHandler();

    if (DEBUG) printf("Argc = %d\n", argc);
    if (argc > 1) {
        if (!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])) {
            printf(HELP_TEXT);
        }
    }

    sockfd = tcp_listen();
    if (sockfd < 0) {
        if (DEBUG) printf("server main > tcp_listen error\n");
        return 1;
    }

    if(DEBUG) {
        inet_ntop(AF_INET, &(servaddr.sin_addr), ip, INET_ADDRSTRLEN);
        printf("server > main > Server IP = %s\n", ip);
    }

    while (!process_endProcess()) {
        /*/ Accept the data packet from client and verification/*/
        connfd = tcp_accept(sockfd);
        if (connfd < 0) {
            if (DEBUG) {
                if (process_endProcess())
                    printf("server main > tcp_accept interrupted by Ctrl+C\n");
                else
                    printf("server main > tcp_accept error\n");
            }

            close(sockfd);
            return 2;
        }
        if (!process_endProcess()) {
            if (DEBUG) {
                printf("##############################\n");
                printf("//                         //\n");
                printf("//  Peticion -> Connfd = %d //\n", connfd);
                printf("//                         //\n");
                printf("##############################\n");
            }

            process_request(connfd);
        }
    }

    /*/ After chatting close the socket/*/
    close(sockfd);
    return 0;
}
