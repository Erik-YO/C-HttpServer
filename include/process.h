

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define METHOD_GET 224
#define METHOD_POST 326
#define METHOD_OPTIONS 556

#define GET "GET"
#define POST "POST"
#define OPTIONS "OPTIONS"

#define NOT_SUPPORTED_VERB "Verbo no soportado\r\r\n"
#define DEFAULT_RESPONSE "Peticion devuelta"

void process_request(int connfd);

void process_setSignalHandler();

int process_endProcess();

#endif
