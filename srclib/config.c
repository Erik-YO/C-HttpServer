

#include "config.h"

#include <stdio.h>

#include "types.h"

/* Default values */
static volatile int numThreads = 1;

/* Read from config file - Execute on initialization */
int config_initFromFile(char *filename) {
    FILE *f = NULL;

    f = fopen(filename, "r");
    if (!f) {
        if (DEBUG) printf("config > initFromFile > fopen error\n");
        return -1;
    }

    /* Parseo del fichero */

    fclose(f);
    return 0;
}

/* Print configuration file help */
void config_printHelp(FILE *f) {
    if (!f) {
        f = stdout;
    }

    fprintf(f, "\n");

    return;
}

/* Get functions */
/* (int/char/...) config_get...(); */
