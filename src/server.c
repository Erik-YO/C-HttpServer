
#include <arpa/inet.h> /*Para inet_ntop*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "hilos.h"
#include "process.h"
#include "tcp.h"
#include "types.h"
#define MAX 8000
#define MAX_STR 50

#define HELP_TEXT \
    "\
\nHelp:\n\
\tCreated by: Erik Yuste <erik.yuste@estudiante.uam.es> & Shenxiao Lin <shenxiao.lin@estudiante.uam.es>\n\
\tCtrl + C to stop the server\n\
\n\
"

/*/ Driver function/*/
int main(int argc, char const *argv[]) {
    int sockfd, connfd, err, max_hilos;
    struct sockaddr_in servaddr;
    char ip[MAX_STR];
    char auxstr[MAX_STR];
    FILE *f = NULL;
    GestorHilos *gh = NULL;
    ip[0] = (char)0;

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

    max_hilos = config_default_max_threads();

    gh = hilo_getGestor(max_hilos);
    if (!gh) {
        if (config_debug()) fprintf(config_debug_file(), "server main > hilo_getGestor error\n");
        config_close_debug_file();
        return 10;
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

    /* Imprime el directorio actual */
    if (config_debug()) {
        fprintf(config_debug_file(), "server > pwd (current working directory) > '");
        f = popen("pwd", "r");
        if (f) {
            while (fgets(auxstr, MAX_STR, f) != NULL) fprintf(config_debug_file(), "%s", auxstr);
            pclose(f);
        }
        fprintf(config_debug_file(), "'\n");
    }

    while (!process_endProcess()) {
        /* Accept the data packet from client and verification */

        /*Esperar hasta que haya algun hilo disponible*/
        hilos_waitUntilAvailable(gh);
        connfd = tcp_accept(sockfd);
        if (connfd < 0) {
            if (config_debug()) {
                if (process_endProcess()) {
                    fprintf(config_debug_file(), "server main > tcp_accept interrupted by Ctrl+C\n");
                } else {
                    fprintf(config_debug_file(), "server main > tcp_accept error\n");
                }
            }

            hilo_destroyGestor(gh);
            close(sockfd);
            config_close_debug_file();
            return 3;
        }

        if (!process_endProcess()) {
            if (config_debug()) {
                fprintf(config_debug_file(), "##############################\n");
                fprintf(config_debug_file(), "//                         //\n");
                fprintf(config_debug_file(), "//  Peticion -> Connfd = %d //\n", connfd);
                fprintf(config_debug_file(), "//  entrante               //\n");
                fprintf(config_debug_file(), "##############################\n");
            }

            /*habria que considerar qué hilos están libres*/
            if ((max_hilos - hilo_getActive(gh)) < 1) {
                if (config_debug()) fprintf(config_debug_file(), "Ya no ningun hilo libre\n");

            } else {
                /*process_request es la funcion principal de proceso de peticiones*/
                err = hilo_launch(gh, (funcionHilo)process_request, (void *)(intptr_t)connfd); /*con el casting (void*)connfd da warning*/
                if (err) {
                    if (config_debug()) fprintf(config_debug_file(), "server main > hilo_launch error\n");
                    hilo_destroyGestor(gh);
                    close(sockfd);
                    break;
                }
            }
        } else {
            close(sockfd);
        }
    }

    if (config_debug()) fprintf(config_debug_file(), "server > main > endWhile\n");

    hilo_destroyGestor(gh);
    /* Cierre del socket y el fichero de debug */
    config_close_debug_file();
    return 0;
}
