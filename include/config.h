

#ifndef CONFIG_H
#define CONFIG_H

/*
 == 0 -> OK
 != 0 -> ERROR
*/
int config_initFromFile(char* filename);

void config_printHelp(FILE* f);

#endif
