

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* Codigos arbitrarios */
#define METHOD_GET 9224
#define METHOD_POST 9326
#define METHOD_OPTIONS 9556

#define GET "GET"
#define POST "POST"
#define OPTIONS "OPTIONS"

void process_request(int connfd);

void process_setSignalHandler();

int process_endProcess();

#endif
