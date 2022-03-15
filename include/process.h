

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"


#define GET "GET"
#define POST "POST"
#define OPTIONS "OPTIONS"

/*
 * FUNCION: void process_request(int connfd)
 * ARGS_IN: int - descriptor del socket por el que mandar la respuesta
 * DESCRIPCION: procesa las peticiones http que lleguen por el socket y las responden 
 */
void process_request(int connfd);

/*
 * FUNCION: void process_setSignalHandler()
 * DESCRIPCION: establece el manejador de senales de la senal SIGINT
 */
void process_setSignalHandler();

/*
 * FUNCION: int process_endProcess()
 * DESCRIPCION: comprueba si process ha recibido la senal SIGINT o no
 * ARGS_OUT: int - resultado: 1 senal recibida, 0 senal no recibida
 */
int process_endProcess();

#endif
