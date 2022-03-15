

#include "process.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "errno.h"
#include "picohttpparser.h"
#include "types.h"
#define MAX_BUF 8000
#define MAX_CONTENT 4000
#define MAX_STR 90
#define METHOD_STR_LEN 10
#define MAX_HEADERS 100
/* Maxima longitud de argumentos */
#define MAX_ARGS 100
#define PATH_STR_LEN 100
#define MAX_CMD_LEN 300

/* Interpretes y extensiones */
#define PYTHON_INT "python3"
#define PHP_INT "php"
#define PYTHON_EXT ".py"
#define PHP_EXT ".php"

/* Recurso por defecto */
#define DEFAULT_RESOURCE "index.html"

/* Codigos arbitrarios */
#define METHOD_GET 9224
#define METHOD_POST 9326
#define METHOD_OPTIONS 9556

/* almacena toda la informacion de una peticion http */
typedef struct {
    /*Cabeceras*/
    char headers[MAX_HEADERS][MAX_STR];
    char headerVal[MAX_HEADERS][MAX_STR];
    int numHeaders;

    /*Metodo: GET, POST, ...*/
    char method[METHOD_STR_LEN];

    /*Ruta*/
    char path[PATH_STR_LEN];

    /* Http version = 1.<version> */
    int version;

    /*Longitud total en Bytes de request (sin contenido)*/
    int total_len;

    /*keep-alive*/
    int keep_alive;

    /*longitud total del contenido de la peticion*/
    long content_len;

} http_req;

/* Extensiones asociadas a cada etiqueta Content-Type*/
struct {
    char *extension;
    char *content_type;
} extension_type[] = {{".txt", "text/plain"},
                      {".htm", "text/html; charset=utf-8"},
                      {".html", "text/html"},
                      {".gif", "image/gif"},
                      {".jpg", "image/jpeg"},
                      {".jpeg", "image/jpeg"},
                      {".ico", "image/ico"},
                      {".mpeg", "video/mpeg"},
                      {".mpg", "video/mpeg"},
                      {".doc", "application/msword"},
                      {".docx", "application/msword"},
                      {".pdf", "application/pdf"},
                      {"-1", "-1"}};

#define DEFAULT_EXTENSION_TYPE "text/plain"

/* http status codes */
#define SC_OK 200

#define SC_BAD_REQUEST 400
#define SC_UNAUTHORIZED 401
#define SC_FORBIDDEN 403
#define SC_NOT_FOUND 404
#define SC_METHOD_NOT_ALLOWED 405
#define SC_URI_TOO_LONG 414

#define SC_INTERNAL_SERVER_ERROR 500
#define SC_METHOD_NOT_IMPLEMENTED 501
#define SC_SERVICE_UNAVAILABLE 503

/* Definitions */

/*
 * FUNCION: int process_parse_request(http_req *data, char *buf, int len)
 * ARGS_IN: http_req* - estructura en la que se guardan los datos de la peticion 
 *          char* - buffer con el texto de la peticion
 *          int - tamano del texto de la peticion
 * DESCRIPCION: procesa una peticion y guarda los campos en en la estructura data
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int process_parse_request(http_req *data, char *buf, int len);

/*
 * FUNCION: int get_method_code(char *mstr)
 * ARGS_IN: char* - cadena con el nombre del metodo 
 * DESCRIPCION: obtiene el codigo del metodo asociado al nombre del metodo
 * ARGS_OUT: int - resultado: codigo del metodo (definido en una macro)
 */
int get_method_code(char *mstr);

/*
 * FUNCION: int process_get(http_req *data, int connfd)
 * ARGS_IN: http_req* - estructura con los datos de la peticion GET
 *          int - descriptor del socket por el que mandar la respuesta 
 * DESCRIPCION: procesa una peticion GET
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int process_get(http_req *data, int connfd);

/*
 * FUNCION: int process_post(http_req *data, int connfd, const char *contenido)
 * ARGS_IN: http_req* - estructura con los datos de la peticion POST
 *          int - descriptor del socket por el que mandar la respuesta
 *          const char* - contenido de la peticion POST 
 * DESCRIPCION: procesa una peticion POST
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int process_post(http_req *data, int connfd, const char *contenido);

/*
 * FUNCION: int process_options(http_req *data, int connfd)
 * ARGS_IN: http_req* - estructura con los datos de la peticion OPTIONS
 *          int - descriptor del socket por el que mandar la respuesta
 * DESCRIPCION: procesa una peticion OPTIONS
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int process_options(http_req *data, int connfd);

/*
 * FUNCION: int send_http_error_response(int connfd, int errorCode)
 * ARGS_IN: int - descriptor del socket por el que mandar la respuesta
 *          int - codigo de error
 * DESCRIPCION: envia una respuesta de error
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int send_http_error_response(int connfd, int errorCode);

/*
 * FUNCION: int get_datetime(char *date)
 * ARGS_IN: char* - puntero a char donde se guardara la fecha
 * DESCRIPCION: guarda la fecha actual formateada en una cadena
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int get_datetime(char *date);

/*
 * FUNCION: int strends(char *a, char *b)
 * ARGS_IN: char* - primera cadena
 *          char* - segunda cadena
 * DESCRIPCION: comprueba si el final de la primera cadena es la segunda
 * ARGS_OUT: int - resultado: 1 en caso afirmativo, 0 en otro caso
 */
int strends(char *a, char *b);

/*
 * FUNCION: void get_content_type(char *path, char *etype)
 * ARGS_IN: char* - direccion del fichero
 *          char* - cadena donde se guardara el tipo de media (html, text, imagen, pdf)
 * DESCRIPCION: obtiene el tipo de contenido segun la extension del archivo (segun la estructura "extension_type")
 */
void get_content_type(char *path, char *etype);

/*
 * FUNCION: long get_file_size(FILE *f)
 * ARGS_IN: FILE* - fichero
 * DESCRIPCION: obtinene el tamano total del fichero
 * ARGS_OUT: long - resultado: tamano del fichero en Bytes, 0 en caso de error
 */
long get_file_size(FILE *f);

/*
 * FUNCION: int get_ruta_completa(char *path, char *ruta)
 * ARGS_IN: char* - ruta relativa al recurso (desde el directorio del servidor)
 *          char* - output de la ruta completa al recurso
 * DESCRIPCION: obtiene la ruta completa del recurso
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int get_ruta_completa(char *path, char *ruta);

/*
 * FUNCION: int get_ruta_completa(char *path, char *ruta)
 * ARGS_IN: char* - ruta relativa al recurso (desde el directorio del servidor)
 *          char* - output de la ruta completa al recurso
 * DESCRIPCION: obtiene la ruta completa del recurso
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int get_argumentos(char *path, char *args);

/*
 * FUNCION: FILE *get_recurso(int connfd, char *ruta, int execute)
 * ARGS_IN: int - descriptor del socket por el que mandar la respuesta
 *          char* - ruta del recurso a obtener o comando a ejecutar
 *          int - flag que decide ejecutar o no un comando (0 obtener fichero, 1 ejecutar comando)
 * DESCRIPCION: obtiene el recurso pedido en un fichero
 * ARGS_OUT: FILE* - resultado: puntero al fichero en el que se ha escrito el resultado
 */
FILE *get_recurso(int connfd, char *ruta, int execute);

/*
 * FUNCION: int send_recurso(int connfd, int version, char *method, char *content_type, FILE *contentf)
 * ARGS_IN: int - descriptor del socket por el que mandar la respuesta
 *          int - version de http
 *          char* - tipo de contenido a enviar
 *          FILE* - puntero del fichero que contiene el 
 * DESCRIPCION: obtiene el recurso pedido en un fichero y lo envia por el socket
 * ARGS_OUT: int - resultado: 0 con ejecucion correcta, < 0 en caso contrario
 */
int send_recurso(int connfd, int version, char *content_type, FILE *contentf);

/* Variables de control */
static volatile int endProcess = FALSE;

/* Manejador de senales */
void manejador(int sig) {
    if (config_debug()) fprintf(config_debug_file(), "process > manejador > signal = %d\n", sig);
    if (sig == SIGINT) endProcess = TRUE;
}

void process_setSignalHandler() {
    struct sigaction act;

    act.sa_handler = manejador;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;

    if (sigaction(SIGINT, &act, NULL) < 0) {
        endProcess = TRUE;
    }
}

int process_endProcess() { return endProcess; }

/* Functions */

int process_parse_request(http_req *data, char *buf, int len) {
    int err, i;
    size_t prevbuflen = 0, numHead = MAX_HEADERS, mlen = 0, plen = 0;
    struct phr_header headers[MAX_HEADERS];
    const char *method, *path;

    if (!data || !buf) return -4;

    /* picohttpparser */
    err = phr_parse_request(buf, len, &(method), &(mlen), &(path), &(plen), &(data->version), headers, &(numHead), prevbuflen);

    if (err < 0) return err; /* En caso de error: err = -1 o -2 */
    if (sizeof(buf) == len) return -3;

    /* Guardar valores en data */
    data->content_len = 0;
    data->numHeaders = (int)numHead;                            /* Numero de cabeceras */
    sprintf(data->method, "%.*s", (int)mlen, method);           /* metodo */
    sprintf(data->path, "%.*s", (int)plen, path);               /* ruta */
    for (i = 0; i < data->numHeaders && i < MAX_HEADERS; i++) { /* cabeceras */
        sprintf(data->headers[i], "%.*s", (int)headers[i].name_len, headers[i].name);
        sprintf(data->headerVal[i], "%.*s", (int)headers[i].value_len, headers[i].value);

        if(!strcmp("Content-Length", data->headers[i])){
            data->content_len = strtol(data->headerVal[i], NULL, 10);
        }

    }
    data->total_len = err; /* Longitud total de la peticion (bytes consumidos por phr_parse_request) */
    data->keep_alive = FALSE;
    if (data->version > 0) { /* Keep-Alive */
        for (i = 0; i < data->numHeaders; i++) {
            if (!strstr(data->headers[i], "Connection")) {
                if (!strstr(data->headerVal[i], "keep-alive")) data->keep_alive = TRUE;
                if (!strstr(data->headerVal[i], "close")) i = data->numHeaders;
            }
        }
    }

    /*DEBUG*/
    if (config_debug()) {
        fprintf(config_debug_file(), "\nprocess > process_parse_request:\n");

        fprintf(config_debug_file(), "\tRequest total len = %d bytes\n", data->total_len);
        fprintf(config_debug_file(), "\tMethod = '%s'\n", data->method);
        fprintf(config_debug_file(), "\tPath = '%s'\n", data->path);
        fprintf(config_debug_file(), "\tHTTP version = 1.%d\n", data->version);
        fprintf(config_debug_file(), "\tkeep-alive = %d\n", data->keep_alive);
        fprintf(config_debug_file(), "\tHeaders:\n");
        for (i = 0; i < data->numHeaders; i++) {
            fprintf(config_debug_file(), "\t\t%s: %s\n", data->headers[i], data->headerVal[i]);
        }
    }

    return 0;
}

/* Main processing function */
void process_request(int connfd) {
    char buf[MAX_BUF];
    int n = 0, err = 0, verbo = 0;
    http_req data;
    buf[0] = (char)0;

    if (connfd < 0) return;

    signal(SIGPIPE, SIG_IGN); /* Ignoramos las senales de broken pipe, pues haremos la comprobacion manualmente */

    do {
        /* Inicia el buffer a '\0' */
        if (!endProcess) {
            bzero(buf, MAX_BUF);
            /* Se lee la peticion del buffer */
            n = 0;
            while ((n += read(connfd, buf, sizeof(buf))) == -1 && errno == EINTR && !endProcess)
                ;

            if (n <= 0) {
                if (config_debug() && n < 0) fprintf(config_debug_file(), "\nprocess > process_request > buffer read error %d\n", n);
                close(connfd);
                return;
            }
        }
        if (config_debug() && !endProcess)
            fprintf(config_debug_file(), "\nprocess > process_request > buffer:\n##----##\n%s##----##\n", buf);

        /*
         *  START PARSING
         */
        /* Parse http request */
        if (!endProcess) {
            err = process_parse_request(&data, buf, n);
            if (config_debug() && !err) fprintf(config_debug_file(), "\nprocess_request > verbo -> '%s'\n", data.method);
        }

        /*Error parsing*/
        if (!endProcess && err) {
            if (config_debug() && !endProcess) fprintf(config_debug_file(), "process > process_request error = %d\n", err);
            send_http_error_response(connfd, SC_BAD_REQUEST);
            close(connfd);
            return;
        }
        /*
         *  END PARSING
         */

        /*
         * START PROCESSING
         */
        if (!endProcess) {
            verbo = get_method_code(data.method); /* Obtenemos el codigo numerico del metodo */
            switch (verbo) {                      /* Llamamos a la funcion que corresponda */
                case METHOD_GET:
                    err = process_get(&data, connfd);
                    break;
                case METHOD_POST:
                    err = process_post(&data, connfd, buf + data.total_len);
                    break;
                case METHOD_OPTIONS:
                    err = process_options(&data, connfd);
                    break;

                default: /* Si el verbo no esta implementado se manda el codigo de error correspondiente */
                    /* Verbo no soportado */
                    send_http_error_response(connfd, SC_METHOD_NOT_IMPLEMENTED);

                    if (config_debug()) fprintf(config_debug_file(), "\nprocess_request > verbo no soportado %s\n", data.method);
                    /* Cierre del descriptor de fichero */
                    close(connfd);
                    return;
            }

            if (config_debug()) fprintf(config_debug_file(), "process_request > process_%s() = %d\n", data.method, err);
        }
        /*
         * END PROCESSING
         */

        /* Suponemos que si err != 0, ya se ha enviado el mensaje de error correspondiente desde la funcion process_<method>() */
    } while (data.keep_alive && !endProcess && !err && FALSE); /*DEBUG*/

    /* Respuesta estandar de servidor en mantenimiento (apagandose) */
    if (endProcess) {
        send_http_error_response(connfd, SC_SERVICE_UNAVAILABLE);
    } else {
        if (config_debug()) fprintf(config_debug_file(), "process_request > conexion file descriptor closed\n");
    }

    /* Se cierra el descriptor de fichero de conexion */
    close(connfd);

    return;
}

/*
 * Funciones de los verbos soportados
 */
/* GET */
/*obtiene un recurso del servidor*/
int process_get(http_req *data, int connfd) {
    char ruta_recurso[PATH_STR_LEN + MAX_STR], etype[MAX_STR];
    char argumentos[MAX_ARGS], script_ejecucion[MAX_CMD_LEN];
    int err = 0;
    FILE *salida = NULL;

    if (!data || connfd < 0) {
        if (config_debug()) fprintf(config_debug_file(), "process > process_get > error en argumentos\n");
        return -1;
    }

    /* Obtenemos la ruta del recurso */
    err = get_ruta_completa(data->path, ruta_recurso);
    if (config_debug()) fprintf(config_debug_file(), "process > process_get > ruta de recurso '%s', err = %d\n", ruta_recurso, err);

    if (access(ruta_recurso, F_OK) != 0) {              /* Comprobamos que el recurso exista en la ruta */
        send_http_error_response(connfd, SC_NOT_FOUND); /* Mensaje de error (no encontrado) */
        if (config_debug()) fprintf(config_debug_file(), "process > process_get > recurso NO EXISTE '%s'\n", ruta_recurso);
        return -2;
    }
    if (access(ruta_recurso, R_OK) != 0) {              /* Comprobamos que se tengan permisos de lectura */
        send_http_error_response(connfd, SC_FORBIDDEN); /* Mensaje de error (acceso denegado) */
        if (config_debug()) fprintf(config_debug_file(), "process > process_get > recurso sin permiso de lectura '%s'\n", ruta_recurso);
        return -3;
    }

    /* Obtenemos argumentos (separados por espacios) si los tiene */
    err = get_argumentos(data->path, argumentos);
    if (config_debug()) fprintf(config_debug_file(), "process > process_get > argumentos '%s', err = %d\n", argumentos, err);

    /* Identificar el tipo de script (si es un scrip a ejecutar) y ejecutar (o leer)*/
    if (strends(ruta_recurso, PYTHON_EXT)) { /* Python */
        sprintf(script_ejecucion, "%s %s %s", PYTHON_INT, ruta_recurso, argumentos);
        salida = get_recurso(connfd, script_ejecucion, TRUE);

    } else if (strends(ruta_recurso, PHP_EXT)) { /* Php */
        sprintf(script_ejecucion, "%s %s %s", PHP_INT, ruta_recurso, argumentos);
        salida = get_recurso(connfd, script_ejecucion, TRUE);

    } else { /* Recurso a leer */
        salida = get_recurso(connfd, ruta_recurso, FALSE);
    }

    /* Suponemos que ya se han enviado los mensajes correspondientes de error al cliente*/
    if (!salida) {
        return -4;
    }

    get_content_type(ruta_recurso, etype); /* Obtenemos el Content-type */

    /* Enviamos la respuesta */
    err = send_recurso(connfd, data->version, etype, salida);

    /* Cerramos el fichero de contenido */
    fclose(salida);

    return err;
}

/* POST */
/*para enviar una entidad a un recurso especificado, provocando cambios en el estado o en el servidor*/
int process_post(http_req *data, int connfd, const char *contenido) {
    char ruta_recurso[PATH_STR_LEN + MAX_STR], etype[MAX_STR];
    char argumentos[MAX_ARGS], script_ejecucion[MAX_CMD_LEN], content[MAX_CONTENT];
    int err = 0, i=0, j=0;
    FILE *salida = NULL;

    if (!data || connfd < 0) {
        if (config_debug()) fprintf(config_debug_file(), "process > process_get > error en argumentos\n");
        return -1;
    }

    /* Obtenemos la ruta del recurso */
    err = get_ruta_completa(data->path, ruta_recurso);
    if (config_debug()) fprintf(config_debug_file(), "process > process_get > ruta de recurso '%s', err = %d\n", ruta_recurso, err);

    if (access(ruta_recurso, F_OK) != 0) {              /* Comprobamos que el recurso exista en la ruta */
        send_http_error_response(connfd, SC_NOT_FOUND); /* Mensaje de error (no encontrado) */
        if (config_debug()) fprintf(config_debug_file(), "process > process_get > recurso NO EXISTE '%s'\n", ruta_recurso);
        return -2;
    }
    if (access(ruta_recurso, R_OK) != 0) {              /* Comprobamos que se tengan permisos de lectura */
        send_http_error_response(connfd, SC_FORBIDDEN); /* Mensaje de error (acceso denegado) */
        if (config_debug()) fprintf(config_debug_file(), "process > process_get > recurso sin permiso de lectura '%s'\n", ruta_recurso);
        return -3;
    }

    /* Obtenemos argumentos (separados por espacios) si los tiene */
    err = get_argumentos(data->path, argumentos);
    if (config_debug()) fprintf(config_debug_file(), "process > process_get > argumentos '%s', err = %d\n", argumentos, err);

    /* Obtenemos el contenido (sus argumentos) */
    while(contenido[i] != '=' && i<data->content_len) i++; /* nombre de la primera variable */
    while(i<data->content_len){
        if(contenido[i] == '='){ /* inicio nuevo argumento*/
            if(j){ /* no se incluye salto de linea si es el primero */
                content[j] = '\n';
                j++;
            }
        }else if(contenido[i] == '&'){
            while(contenido[i] != '=' && i<data->content_len) i++; /* nombre de variable */
            i--;
        }else{
            content[j] = contenido[i];
            j++;
        }
        i++;
    }
    content[j] = (char)0; /* fin del contenido */


    /* Identificar el tipo de script (si es un scrip a ejecutar) y ejecutar (o leer)*/
    if (strends(ruta_recurso, PYTHON_EXT)) { /* Python */
        if(data->content_len>0){/* con argumentos por stdin */
            sprintf(script_ejecucion, "echo \"%s\" | %s %s %s", content, PYTHON_INT, ruta_recurso, argumentos);
        }else{ /* sin argumentos */
            sprintf(script_ejecucion, "%s %s %s", PYTHON_INT, ruta_recurso, argumentos);
        }
        salida = get_recurso(connfd, script_ejecucion, TRUE);

    } else if (strends(ruta_recurso, PHP_EXT)) { /* Php */
        if(data->content_len>0){/* con argumentos por stdin */
            sprintf(script_ejecucion, "echo \"%s\" | %s %s %s", content, PHP_INT, ruta_recurso, argumentos);
        }else{
            sprintf(script_ejecucion, "%s %s %s", PHP_INT, ruta_recurso, argumentos);
        }
        salida = get_recurso(connfd, script_ejecucion, TRUE);

    } else { /* Recurso a leer */
        salida = get_recurso(connfd, ruta_recurso, FALSE); /* No se tiene en cuenta el contenido */
    }

    /* Suponemos que ya se han enviado los mensajes correspondientes de error al cliente*/
    if (!salida) {
        return -4;
    }

    get_content_type(ruta_recurso, etype); /* Obtenemos el Content-type */

    /* Enviamos la respuesta */
    err = send_recurso(connfd, data->version, etype, salida);

    /* Cerramos el fichero de contenido */
    fclose(salida);

    return err;
}

/* OPTIONS */
/*para describir las opciones de comunicacion del recurso destino*/
int process_options(http_req *data, int connfd) {
    int err;

    if (!data || connfd < 0) {
        if (config_debug()) fprintf(config_debug_file(), "process > process_options > error en los argumentos\n");
        close(connfd);
        return -1;
    }

    err = send_recurso(connfd, data->version, NULL, NULL);
    if (err < 0) {
        if (config_debug()) fprintf(config_debug_file(), "process > process_options > error en send_recurso\n");
        return -2;
    }

    close(connfd);

    return 0;
}

/*
 * Funciones Privadas (auxiliares sencillas)
 */

/* Verbos soportados -> Seleccionar verbo */
int get_method_code(char *mstr) {
    if (!strcmp(mstr, GET)) return METHOD_GET;
    if (!strcmp(mstr, POST)) return METHOD_POST;
    if (!strcmp(mstr, OPTIONS)) return METHOD_OPTIONS;

    /* Verbo/Metodo no soportado */
    return 0;
}

int get_datetime(char *date) {
    time_t t;
    struct tm *time_now;

    if(!date) return -1;
    date[0] = (char)0;

    t = time(NULL);
    time_now = localtime(&t);

    /*construir el string de la fecha*/
    strftime(date, MAX_STR, "%a %d-%b-%Y %H:%M:%S GMT %z", time_now);

    return 0;
}

/*Si a termina con b*/
int strends(char *a, char *b) {
    char *s;
    if (!a || !b) return FALSE;

    s = strrchr(a, b[0]); /* Se busca el ultimo punto */

    if (s != NULL) {
        return !strcmp(s, b);
    }

    return FALSE;
}

/*Obtener el tipo de contenido*/
void get_content_type(char *path, char *etype) {
    int i = 0, found = 0;

    if (!etype) return;
    if (!path) {
        if (config_debug()) fprintf(config_debug_file(), "process > process_get_content_type > path = NULL\n");
        sprintf(etype, "%s", DEFAULT_EXTENSION_TYPE);
        return;
    }

    /* recorre toda la estructura que contiene todos los tipos con el while */
    /* para cuando identifica uno de ellos o cuando llega al final*/
    while (strcmp(extension_type[i].extension, "-1") && !found) {
        if (strends(path, extension_type[i].extension)) {
            sprintf(etype, "%s", extension_type[i].content_type);
            found = 1;
        }
        i++;
    }

    /* Por defecto texto plano */
    if (!found) {
        if (config_debug()) fprintf(config_debug_file(), "process > process_get_content_type > tipo no soportado '%s'\n", path);
        sprintf(etype, DEFAULT_EXTENSION_TYPE);
    }

    return;
}

int send_http_error_response(int connfd, int errorCode) {
    char date[MAX_STR], errName[MAX_STR];
    char headers[MAX_BUF], content[MAX_CONTENT];
    int i = 0, clen, err;

    if (connfd < 0 || errorCode < 100) {
        if (config_debug()) {
            fprintf(config_debug_file(), "process > send_http_error_response > argument(s) error connfd=%d, errorCode=%d\n", connfd,
                    errorCode);
        }
        return -1;
    }

    err = get_datetime(date);
    if (err) {
        if (config_debug()) fprintf(config_debug_file(), "process > send_http_error_response > get_datetime error = %d\n", err);
        return -2;
    }

    switch (errorCode) {
        /*4xx*/
        case SC_BAD_REQUEST:
            sprintf(errName, "Bad request");
            break;
        case SC_UNAUTHORIZED:
            sprintf(errName, "Unauthorized");
            break;
        case SC_FORBIDDEN:
            sprintf(errName, "Forbidden");
            break;
        case SC_NOT_FOUND:
            sprintf(errName, "Not Found");
            break;
        case SC_METHOD_NOT_ALLOWED:
            sprintf(errName, "Method Not Allowed");
            break;
        case SC_URI_TOO_LONG:
            sprintf(errName, "Uri Too long");
            break;

        /*5xx*/
        case SC_INTERNAL_SERVER_ERROR:
            sprintf(errName, "Internal Server Error");
            break;
        case SC_METHOD_NOT_IMPLEMENTED:
            sprintf(errName, "Method Not Implemented");
            break;
        case SC_SERVICE_UNAVAILABLE:
            sprintf(errName, "Service Unavailable");
            break;

        default:
            sprintf(errName, "Undefined Error");
            break;
    }

    /* Preparamos el contenido (pagina html con el error) */
    clen = sprintf(content, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
    clen += sprintf(&(content[clen]), "<html><head>\n");
    clen += sprintf(&(content[clen]), "<title>%d %s</title>\n", errorCode, errName);
    clen += sprintf(&(content[clen]), "</head><body>\n");
    clen += sprintf(&(content[clen]), "<h1>%s</h1>\n", errName);
    clen += sprintf(&(content[clen]), "</body></html>\n");

    /* Preparamos las cabeceras */
    i = sprintf(headers, "HTTP/1.1 %d %s\r\n", errorCode, errName);
    i += sprintf(&(headers[i]), "Date: %s\r\n", date);
    i += sprintf(&(headers[i]), "Server: %s\r\n", config_server_signature());
    i += sprintf(&(headers[i]), "Allow: GET, POST, OPTIONS\r\n");
    i += sprintf(&(headers[i]), "Content-Length: %d\r\n", clen);
    i += sprintf(&(headers[i]), "Connection: close\r\n");
    i += sprintf(&(headers[i]), "Content-Type: %s\r\n", "text/html");
    i += sprintf(&(headers[i]), "\r\n");

    /* Enviamos las cabeceras */
    err = send(connfd, headers, i * sizeof(char), 0);
    if (err < 0) return -3;

    /* Enviamos el contenido */
    err += send(connfd, content, clen * sizeof(char), 0);
    if (err < 0) return -4;

    return 0;
}

/* Del campo path se obtiene la ruta completa al recurso */
int get_ruta_completa(char *path, char *ruta) {
    int l=0, i = 0, rootLen=0;

    if (!path || !ruta) return -1;

    ruta[0] = (char)0;
    l = strlen(path);
    rootLen = sprintf(ruta, "%s", config_server_root());

    while (i < l) {
        if (path[i] == '?') { /* El caracter '?' delimita la ruta (donde empiezan los argumentos) */
            i = l;
        } else {
            ruta[i + rootLen + 1] = ruta[i + rootLen]; /* Se traslada el caracter final una posicion adelante */
            ruta[i + rootLen] = path[i];               /* Se copia el caracter de path */
        }

        i++;
    }

    /* Si la ruta es "/" se obtiene el recurso por defecto: index.html */
    if (ruta[strlen(ruta) - 1] == '/') {
        sprintf(&(ruta[strlen(ruta)]), DEFAULT_RESOURCE);
    }

    return 0;
}

/* Se obtienen los argumentos del campo path */
int get_argumentos(char *path, char *args) {
    int l, i = 0, j = 0;
    if (!path || !args) return -1;

    l = strlen(path);
    args[0] = (char)0;

    while (i < l && path[i] != '?') i++;

    i++;
    /* Obtenemos los parametros separados por espacios */
    while (i < l) {
        while (i < l && path[i] != '=') {
            i++;
        }
        i++;
        while (i < l && path[i] != '&') {
            args[j] = path[i];
            /*sprintf(args, "%s", &(path[i]));*/
            j++;
            i++;
        }
        args[j] = ' ';
        j++;
    }
    if (j) {
        args[j - 1] = (char)0;
    }

    return 0;
}

long get_file_size(FILE *f) {
    long size, prev;

    if (!f) return 0;

    prev = ftell(f);
    if (fseek(f, 0, SEEK_END)) return -2;
    ;
    size = ftell(f);
    if (fseek(f, prev, SEEK_SET)) return -3;

    return size;
}

/* extraArgs = argumentos separados por '\n' */
/* TODO: modificar para que acepte los argumentos extraArgs por la entrada estandar */
FILE *get_recurso(int connfd, char *ruta, int execute) {
    FILE *salida = NULL, *exeoutput = NULL;
    char auxBytes[MAX_STR];
    int err, j;

    if (!ruta) {
        if (config_debug()) fprintf(config_debug_file(), "process > get_recurso > !ruta\n");
        send_http_error_response(connfd, SC_NOT_FOUND); /* Envio de mensaje de error */
        return NULL;
    }

    if (execute) {
        /* Generamos un fichero temporal donde guardar el resultado del script */
        salida = tmpfile();
        if (!salida) {
            if (config_debug()) fprintf(config_debug_file(), "process > process_get > error en tmpfile\n");
            send_http_error_response(connfd, SC_INTERNAL_SERVER_ERROR); /* Envio de mensaje de error */
            return NULL;
        }

        /* Se crea el 'fichero' (pipe) de donde se obtendran los datos a mandar */
        exeoutput = popen(ruta, "r");
        if (!exeoutput) {
            if (config_debug()) fprintf(config_debug_file(), "process > process_get > error en popen\n");
            send_http_error_response(connfd, SC_INTERNAL_SERVER_ERROR); /* Envio de mensaje de error */
            return NULL;
        }

        j = 0;
        do { /* Pasamos todo el resultado de la ejecucion a un fichero temporal */
            err = fread(auxBytes, sizeof(char), MAX_STR, exeoutput);
            if (err > 0) {
                j += err;
                fwrite(auxBytes, sizeof(char), err, salida);
            }
        } while (err > 0);
        pclose(exeoutput);
        fseek(salida, 0, SEEK_SET); /* Ponemos el puntero al inicio del fichero (para lectura) */

    } else {
        /* Abrimos el recurso pedido */
        salida = fopen(ruta, "rb");
        if (!salida) {
            /* Recurso no disponible */
            send_http_error_response(connfd, SC_NOT_FOUND);
            return NULL;
        }
    }

    return salida;
}

/* Crea la respuesta para cualquiera de los metodos implementados (GET, OPTIONS y OPTIONS) dependiendo de los argumentos */
int send_recurso(int connfd, int version, char *content_type, FILE *contentf) {
    char response[MAX_STR], date[MAX_STR], auxBytes[MAX_STR];
    long content_len;
    int err = 0, j;

    content_len = get_file_size(contentf); /* Obtenemos la longitud del fichero */
    err = get_datetime(date);              /* Obtenemos la marca de tiempo */
    if (err) {
        if (config_debug()) fprintf(config_debug_file(), "process > send_recurso > error en get_datetime\n");
    } else if (content_len < 0) {
        if (config_debug()) fprintf(config_debug_file(), "process > send_recurso > error en get_file_size\n");
        err = -5;
    }
    if (err) {
        send_http_error_response(connfd, SC_INTERNAL_SERVER_ERROR); /* Error en la obtencion de alguno de los valores de la cabecera */
        return -5;
    }

    /* Construccion de la respuesta */
    j = sprintf(response, "HTTP/1.%d %d OK\r\n", version, SC_OK); /* Version y status (correcto) */
    /* Cabeceras */
    j += sprintf(&(response[j]), "Date: %s\r\n", date);                        /* Fecha */
    j += sprintf(&(response[j]), "Server: %s\r\n", config_server_signature()); /* Nombre del servidor */
    if (content_type) {
        j += sprintf(&(response[j]), "Content-Type: %s\r\n", content_type); /* Tipo de contenido */
    }
    j += sprintf(&(response[j]), "Allow: %s, %s, %s\r\n", GET, POST, OPTIONS); /* Metodos permitidos */
    j += sprintf(&(response[j]), "Content-Length: %ld\r\n\r\n", content_len);  /* Longitud del contenido y final */

    if (config_debug()) fprintf(config_debug_file(), "process > send_recurso > cabeceras de response:\n%s\n", response);

    /* Enviamos las cabeceras */
    err = send(connfd, response, j, 0);
    if (err < 0) return -6;

    if (contentf) {
        err = 0;
        /* Vamos enviando el contenido por partes (si no cabe en una) */
        while (!feof(contentf) && j > 0) {
            j = fread(auxBytes, sizeof(char), MAX_STR, contentf);
            if (j < 0) {
                err = -6;
                if (config_debug()) fprintf(config_debug_file(), "process > send_recurso > no se puede responder al cliente\n");
            } else if (j) {
                send(connfd, auxBytes, j, 0);
            }
        }
    }

    return err;
}
