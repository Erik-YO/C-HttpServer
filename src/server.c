
#include <arpa/inet.h> /*Para inet_ntop*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "process.h"
#include "tcp.h"
#include "types.h"
#define MAX 8000
#define SA struct sockaddr

#define HELP_TEXT \
    "\
\nHelp:\n\
\tCtrl + C to stop the server\n\
\n\
"

/*/ Driver function/*/
int main(int argc, char const *argv[]) {
    int sockfd, connfd, err;
    struct sockaddr_in servaddr;
    char ip[50] = "\0";

    /* Inicializar el manejador de senales para los hilos */
    process_setSignalHandler();

    /* Inicializar el modulo de configuracion */
    err = config_initFromFile();
    if (err) {
        fprintf(stderr, "err %d\n", err);
        config_printHelp(stdout);
        return 1;
    }

    if (config_debug()) fprintf(config_debug_file(), "server > main > argc = %d\n", argc);
    /* Imprimir informacion de ayuda */
    if (argc > 1) {
        if (!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])) {
            /* General */
            printf(HELP_TEXT);
            /* Del fichero de configuracion */
            config_printHelp(stdout);
        }
    }

    /* Se abre el socket para empezar a aceptar conexiones */
    sockfd = tcp_listen();
    if (sockfd < 0) {
        if (config_debug()) fprintf(config_debug_file(), "server main > tcp_listen error\n");
        config_close_debug_file();
        return 2;
    }

    /* Imprime la ip del servidor */
    if (config_debug()) {
        servaddr.sin_addr.s_addr = 0;
        inet_ntop(AF_INET, &(servaddr.sin_addr), ip, INET_ADDRSTRLEN);
        fprintf(config_debug_file(), "server > main > Server IP = %s\n", ip);
    }

    while (!process_endProcess()) {
        /* Accept the data packet from client and verification */
        connfd = tcp_accept(sockfd);
        if (connfd < 0) {
            if (config_debug()) {
                if (process_endProcess()) {
                    fprintf(config_debug_file(), "server main > tcp_accept interrupted by Ctrl+C\n");
                } else {
                    fprintf(config_debug_file(), "server main > tcp_accept error\n");
                }
            }

            close(sockfd);
            config_close_debug_file();
            return 3;
        }

        if (!process_endProcess()) {
            if (config_debug()) {
                fprintf(config_debug_file(), "##############################\n");
                fprintf(config_debug_file(), "//                         //\n");
                fprintf(config_debug_file(), "//  Peticion -> Connfd = %d //\n", connfd);
                fprintf(config_debug_file(), "//                         //\n");
                fprintf(config_debug_file(), "##############################\n");
            }

            /* Llamada a la funcion principal de proceso de peticiones */
            process_request(connfd);
        }
    }

    /* Cierre del socket y el fichero de debug */
    close(sockfd);
    config_close_debug_file();
    return 0;
}
