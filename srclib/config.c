
#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CONF_FILE_NAME "./server.conf"
#define COMMENT_SYM '#'
#define ASSIGN_SYM '='
#define CONFIG_NUM_VARIABLES 7
#define MAX_LINE_LEN 80

/* Default values */
#define DEF_SERVERROOT "./www"
#define DEF_MAXCLIENTS 1
#define DEF_LISTENPORT 8080
#define DEF_MAX_THREADS 1
#define DEF_SERVERSIGNATURE "MyServer 1.1"
#define DEF_DEBUG 0


/* Variables names and explanations */
#define SERVER_ROOT "server_root"
#define MAX_CLIENTS "max_clients"
#define LISTEN_PORT "listen_port"
#define SERVER_SIGNATURE "server_signature"
#define MAX_THREADS "max_threads"
#define DEBUG_FILE "debug_file"
#define DEBUG "debug"

static char *CONFIG_VAR_NAMES[CONFIG_NUM_VARIABLES + 1] = {SERVER_ROOT, MAX_CLIENTS, LISTEN_PORT, SERVER_SIGNATURE, MAX_THREADS,
                                                           DEBUG_FILE,  DEBUG,       NULL};

static char *CONFIG_VAR_EXPLAIN[CONFIG_NUM_VARIABLES + 1] = {
    "Path to the files and resources of the server (relative to the configuration file)",
    "Maximum number of concurrent clients. Subsequent connections will be rejected",
    "Port where the server will accept connections",
    "String returned on the \"ServerName\" header",
    "Maximum number of threads to process new connections",
    "Debug messages output file. \"none\" or \"null\" for no output file. \"stderr\" or \"stdout\" for other standar outputs",
    "Print debug messages. Valid values: true/1 or false/0",
    NULL};

static char *CONFIG_VAR_TYPE[CONFIG_NUM_VARIABLES + 1] = {"char*", "int", "int", "char*", "int", "FILE*", "int", NULL};

/* Configurable variables (default values) */
static char server_root[MAX_LINE_LEN]; /* path without final '/' */
static int max_clients;
static int listen_port;
static char server_signature[MAX_LINE_LEN];
static int max_threads;
static FILE *debug_file;
static int debug;

/* Not accesible */
static char debug_file_name[MAX_LINE_LEN];

/* Private definitions */


/*
 * FUNCION: void _spaces(FILE* f, int n)
 * ARGS_IN: FILE* - fichero de impresion
 *          int - numero de espacios a imprimir
 * DESCRIPCION: Imprime n espacios ' ' en el fichero f
 */
void _spaces(FILE *f, int n);

/*
 * FUNCION: int _parse_line(char *line)
 * ARGS_IN: char* - linea a analizar
 * DESCRIPCION: analiza una linea del fichero y asigna el valor a una variable si corresponde
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, 1 en caso contrario
 */
int _parse_line(char *line);

/*
 * FUNCION: int _starts(char *a, char *b)
 * ARGS_IN: char* - cadena en la que buscar
 *          char* - subcadena
 * DESCRIPCION: comprueba si el inicio de la cadena a es igual que b
 * ARGS_OUT: int - resultado: 1 si corresponde, 0 si no
 */
int _starts(char *a, char *b);

/*
 * FUNCION: void _set_default_values()
 * DESCRIPCION: establece los valores por defecto a las variables
 */
void _set_default_values();

/*
 * FUNCION: void _print_timestamp()
 * DESCRIPCION: imprime una marca de tiempo [DD-MM-AAAA hh:mm:ss] en el fichero de debug
 */
void _print_timestamp();

/*
 * FUNCION: void _print_config()
 * DESCRIPCION: imprime los valores de las variables en el fichero de debug
 */
void _print_config();

/*
 * FUNCION: void _remove_tail_whitespaces(char *str)
 * ARGS_IN: char* - cadena de caracteres
 * DESCRIPCION: Elimina los caracteres ' ' (espacio) y '\t' (tabulador) del final de la cadena str
 */
void _remove_tail_whitespaces(char *str);

/* Public functions */

/* Get default functions */
char *config_default_server_root() { return DEF_SERVERROOT; }

int config_default_max_clients() { return DEF_MAXCLIENTS; }

int config_default_listen_port() { return DEF_LISTENPORT; }

char *config_default_server_signature() { return DEF_SERVERSIGNATURE; }

int config_default_max_threads() { return DEF_MAX_THREADS; }

FILE *config_default_debug_file() { return stderr; }

int config_default_debug() { return DEF_DEBUG; }

/* Get functions */
/* (int /char* /...) config_<var>(); */
char *config_server_root() { return server_root; }

int config_max_clients() { return max_clients; }

int config_listen_port() { return listen_port; }

char *config_server_signature() { return server_signature; }

int config_max_threads() { return max_threads; }

FILE *config_debug_file() { return debug_file; }

int config_debug() { return debug; }

/* Read from config file - Execute on initialization */
int config_initFromFile() {
    FILE *f = NULL;
    char line[MAX_LINE_LEN];
    int error = 0;
    int lineNum = 1, llen;

    _set_default_values();

    f = fopen(CONF_FILE_NAME, "r");
    if (!f) {
        return -1;
    }

    /* Arbitrary value */
    debug_file = f;
    sprintf(debug_file_name, "null");

    /* Parseo del fichero */
    while (fgets(line, sizeof(line), f)) {
        llen = strlen(line);
        if(line[llen-1]=='\n'){
            line[llen-1] = '\0';
        }
        if (llen > 0) {
            if (llen > MAX_LINE_LEN) {
                if (debug_file != f && debug_file != NULL && debug_file != stderr && debug_file != stdout) {
                    fclose(debug_file);
                }
                fclose(f);
                return -2;
            }
            if (line[0] != COMMENT_SYM) {
                error = _parse_line(line);
            }
            if (error) {
                fclose(f);
                printf("config > initFromFile > _parse_line error on line %d > \"%s\"\n", lineNum, line);
                
                return -3;
            }
        }
        lineNum++;
    }

    /* Debug file not changed */
    if (debug_file == f) {
        debug_file = config_default_debug_file();
    } else if (debug_file == NULL) {
        if (!strcmp(debug_file_name, "none")) {
            debug = 0;
        } else {
            debug_file = fopen(debug_file_name, "a");
            if (!debug_file) {
                debug = 0;
                fclose(f);
                return -4;
            }
        }
    }

    if (!debug && debug_file && debug_file != stderr && debug_file != stdout) {
        fclose(debug_file);
    }

    _print_timestamp();
    _print_config();

    fclose(f);
    return 0;
}

/* Print configuration file help */
void config_printHelp(FILE *f) {
    int i = 0;

    if (!f) {
        f = stdout;
    }

    fprintf(f, "\nConfig module help:\n\n");


    /* Configuration File */
    _spaces(f, 2);
    fprintf(f, "The variables are read from the file \"./%s\"\n", CONF_FILE_NAME);

    /* Configurable Variables */
    _spaces(f, 2);
    fprintf(f, "Available configuration variables:\n");

    while (CONFIG_VAR_NAMES[i] != NULL && CONFIG_VAR_EXPLAIN[i] != NULL) {
        _spaces(f, 4);
        fprintf(f, "%s", CONFIG_VAR_NAMES[i]);
        _spaces(f, 20 - strlen(CONFIG_VAR_NAMES[i]));
        fprintf(f, "-> %s config_%s():\n", CONFIG_VAR_TYPE[i], CONFIG_VAR_NAMES[i]);
        _spaces(f, 8);
        fprintf(f, "%s\n\n", CONFIG_VAR_EXPLAIN[i]);
        i++;
    }



    fflush(f);

    return;
}

void config_close_debug_file() {
    if (debug_file && debug_file != stdout && debug_file != stderr) {
        fclose(debug_file);
    }
    debug_file = NULL;
    debug = 0;
}

/* Private functions */

void _set_default_values() {
    /*char server_root[MAX_LINE_LEN];*/
    sprintf(server_root, "%s", config_default_server_root());
    /*int max_clients;*/
    max_clients = config_default_max_clients();
    /*int listen_port;*/
    listen_port = config_default_listen_port();
    /*char server_signature[MAX_LINE_LEN];*/
    sprintf(server_signature, "%s", config_default_server_signature());
    /*int max_threads;*/
    max_threads = config_default_max_threads();
    /*FILE *debug_file;*/
    debug_file = config_default_debug_file();
    /*int debug;*/
    debug = config_default_debug();

    /*debug_file_name*/
    sprintf(debug_file_name, "stderr");

    return;
}

/* Prints n spaces (' ') on the file f */
void _spaces(FILE *f, int n) {
    while (n > 0) {
        fprintf(f, " ");
        n--;
    }
}

/* Check if b is in the first part of a. 1=yes, 0=no*/
int _starts(char *a, char *b) {
    int la, lb, i = 0;

    if (!a || !b) return 0;

    la = strlen(a);
    lb = strlen(b);

    if (la < lb) return 0;

    while (i < la && i < lb) {
        if (a[i] != b[i]) return 0;

        i++;
    }

    /* Finnished b */
    if (i == lb && i <= la) {
        return 1;
    }

    return 0;
}

/* Removes trailing whitespaces */
void _remove_tail_whitespaces(char *str) {
    int l = strlen(str);
    char *end;

    if (l <= 0) return;

    end = str + l - 1;

    while (end > str && ((*end) == ' ' || (*end) == '\t')) end--;

    end[1] = '\0';
}

/* Parse a single line. 0=OK*/
int _parse_line(char *line) {
    int i = 0, l = strlen(line), j = 0;

    if (l == 0) return 0;

    /* Elimination of spaces before the actual name */
    while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;

    if (i >= l) return 0;
    if (line[i] == COMMENT_SYM) return 0;

    _remove_tail_whitespaces(line);
    l = strlen(line);

    if (_starts(&(line[i]), SERVER_ROOT)) { /* server_root */
        i += strlen(SERVER_ROOT);

        while (line[i] == ' ' || line[i] == '\t') i++;
        if (i >= l) return 1;

        /* Check '=' */
        if (line[i] != ASSIGN_SYM) return 1;
        i++;

        while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;
        if (i >= l) return 1;

        /* String copy -> without ending '/' */
        j = sprintf(server_root, "%s", &(line[i])) - 1;
        if (server_root[j] == '/') {
            server_root[j] = '\0';
        }

    } else if (_starts(&(line[i]), MAX_CLIENTS)) { /* max_clients */
        i += strlen(MAX_CLIENTS);

        while (line[i] == ' ' || line[i] == '\t') i++;
        if (i >= l) return 1;

        /* Check '=' */
        if (line[i] != ASSIGN_SYM) return 1;
        i++;

        while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;
        if (i >= l) return 1;

        /* String to long -> int */
        max_clients = (int)strtol(&(line[i]), NULL, 10);
        if (max_clients <= 0 || errno == ERANGE) return 1;

    } else if (_starts(&(line[i]), LISTEN_PORT)) { /* listen_port */
        i += strlen(LISTEN_PORT);

        while (line[i] == ' ' || line[i] == '\t') i++;
        if (i >= l) return 1;

        /* Check '=' */
        if (line[i] != ASSIGN_SYM) return 1;
        i++;

        while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;
        if (i >= l) return 1;

        /* String to long -> int */
        listen_port = (int)strtol(&(line[i]), NULL, 10);
        if (listen_port <= 0 || errno == ERANGE) return 1;

    } else if (_starts(&(line[i]), SERVER_SIGNATURE)) { /* server_signature */
        i += strlen(SERVER_SIGNATURE);

        while (line[i] == ' ' || line[i] == '\t') i++;
        if (i >= l) return 1;

        /* Check '=' */
        if (line[i] != ASSIGN_SYM) return 1;
        i++;

        while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;
        if (i >= l) return 1;

        /* String copy */
        sprintf(server_signature, "%s", &(line[i]));

    } else if (_starts(&(line[i]), MAX_THREADS)) { /* max_threads */
        i += strlen(MAX_THREADS);

        while (line[i] == ' ' || line[i] == '\t') i++;
        if (i >= l) return 1;

        /* Check '=' */
        if (line[i] != ASSIGN_SYM) return 1;
        i++;

        while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;
        if (i >= l) return 1;

        /* String to long -> int */
        max_threads = (int)strtol(&(line[i]), NULL, 10);
        if (max_threads <= 0 || errno == ERANGE) return 1;

    } else if (_starts(&(line[i]), DEBUG_FILE)) { /* debug_file */
        i += strlen(DEBUG_FILE);

        while (line[i] == ' ' || line[i] == '\t') i++;
        if (i >= l) return 1;

        /* Check '=' */
        if (line[i] != ASSIGN_SYM) return 1;
        i++;

        while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;
        if (i >= l) return 1;

        /* String copy */
        if (!strcmp("stderr", &(line[i]))) {
            debug_file = stderr;
            sprintf(debug_file_name, "stderr");
        } else if (!strcmp("stdout", &(line[i]))) {
            debug_file = stdout;
            sprintf(debug_file_name, "stdout");
        } else if (!strcmp("none", &(line[i])) || !strcmp("null", &(line[i]))) {
            debug_file = NULL;
            sprintf(debug_file_name, "none");
        } else {
            debug_file = NULL;
            sprintf(debug_file_name, "%s", &(line[i]));
        }

    } else if (_starts(&(line[i]), DEBUG)) { /* debug */
        i += strlen(DEBUG);

        while (line[i] == ' ' || line[i] == '\t') i++;
        if (i >= l) return 1;

        /* Check '=' */
        if (line[i] != ASSIGN_SYM) return 1;
        i++;

        while ((line[i] == ' ' || line[i] == '\t') && i < l) i++;
        if (i >= l) return 1;

        /* Check: true, 1, false, 0 */
        if (!strcmp(&(line[i]), "true") || !strcmp(&(line[i]), "1")) {
            debug = 1;
        } else if (!strcmp(&(line[i]), "false") || !strcmp(&(line[i]), "0")) {
            debug = 0;
        } else {
            return 1;
        }

    } else {
        /* Variable not found */
        /*printf("Not found - \"%s\"", &(line[i]));*/
        return 1;
    }

    return 0;
}

void _print_timestamp() {
    time_t rawtime;
    struct tm *ti;

    if (!debug || !debug_file) {
        return;
    }

    time(&rawtime);
    ti = localtime(&rawtime);

    fprintf(debug_file, "\n\n[%d-%d-%d %d:%d:%d]\n", ti->tm_mday, ti->tm_mon + 1, ti->tm_year + 1900, ti->tm_hour, ti->tm_min, ti->tm_sec);
    fflush(debug_file);
}

void _print_config() {
    if (!debug || !debug_file) {
        return;
    }

    fprintf(debug_file, "\nCurrent server configuration:\n");
    _spaces(debug_file, 2);
    fprintf(debug_file, "%s = '%s'\n", SERVER_ROOT, server_root);
    _spaces(debug_file, 2);
    fprintf(debug_file, "%s = %d\n", MAX_CLIENTS, max_clients);
    _spaces(debug_file, 2);
    fprintf(debug_file, "%s = %d\n", LISTEN_PORT, listen_port);
    _spaces(debug_file, 2);
    fprintf(debug_file, "%s = '%s'\n", SERVER_SIGNATURE, server_signature);
    _spaces(debug_file, 2);
    fprintf(debug_file, "%s = %d\n", MAX_THREADS, max_threads);
    _spaces(debug_file, 2);
    if (debug) {
        fprintf(debug_file, "%s = true\n", DEBUG);
    } else {
        fprintf(debug_file, "%s = false\n", DEBUG);
    }
    _spaces(debug_file, 2);
    fprintf(debug_file, "%s = '%s'\n", DEBUG_FILE, debug_file_name);

    fprintf(debug_file, "\n");
    fflush(debug_file);
}

int _test() {
    char test[MAX_LINE_LEN], *b;

    printf("Test de _start:\n");

    sprintf(test, "debug = ");
    b = "deb";
    printf("<%s> in <%s> \t= %d\n", b, test, _starts(test, b));
    sprintf(test, " debug = ");
    b = "deb";
    printf("<%s> in <%s>\t= %d\n", b, test, _starts(test, b));
    sprintf(test, "debu");
    b = "debug";
    printf("<%s> in <%s>\t= %d\n", b, test, _starts(test, b));
    sprintf(test, "debug");
    b = "debug";
    printf("<%s> in <%s>\t= %d\n", b, test, _starts(test, b));
    sprintf(test, "debug=");
    b = "debug";
    printf("<%s> in <%s>\t= %d\n", b, test, _starts(test, b));

    printf("\nTest de _parse_line:\n");
    sprintf(test, " server_root");
    printf("Parse_line <%s> = %d\n", test, _parse_line(test));
    sprintf(test, "server");
    printf("Parse_line <%s> = %d\n", test, _parse_line(test));
    sprintf(test, "# debug");
    printf("Parse_line <%s> = %d\n", test, _parse_line(test));
    sprintf(test, "debug = ");
    printf("Parse_line <%s> = %d\n", test, _parse_line(test));

    _set_default_values();

    sprintf(test, "# Test configuration file");
    _parse_line(test);

    sprintf(test, "debug=1");
    _parse_line(test);
    sprintf(test, "debug_file= stdout");
    _parse_line(test);
    sprintf(test, "max_clients =2");
    _parse_line(test);
    sprintf(test, "listen_port = 80");
    _parse_line(test);
    sprintf(test, "server_root=  ./ ");
    _parse_line(test);
    sprintf(test, " #Comment");
    _parse_line(test);
    sprintf(test, "#comment");
    _parse_line(test);
    sprintf(test, " server_signature  = Signature 1.1 ");
    _parse_line(test);
    sprintf(test, "max_threads = 3");
    _parse_line(test);

    debug_file = stdout;
    debug = 1;
    _print_timestamp();
    _print_config();

    return 0;
}
