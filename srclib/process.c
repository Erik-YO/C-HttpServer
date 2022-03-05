

#include "process.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>

#include "errno.h"
#include "picohttpparser.h"
#include "types.h"
#define MAX_BUF 8000
#define MAX_STR 90
#define MAX_HEADERS 100

/* almacena toda la informacion de una peticion http */
typedef struct {
    /*Cabeceras*/
    struct phr_header headers[MAX_HEADERS];
    /*Numero de cabeceras*/
    int numHeaders;
    /*Longitud total en Bytes de request*/
    int total_len;
    /*Metodo: GET, POST, ...*/
    const char *method;
    int method_len;
    /*Ruta*/
    const char *path;
    int path_len;
    /* Http version = 1.<version> */
    int version;

} http_req;

/* Definitions */

int process_parse_request(http_req *data, char *buf, int len);
int get_method(char *mstr);
int process_get(http_req *data, int connfd);
int process_post(http_req *data, int connfd);
int process_options(http_req *data, int connfd);

/* Variables de control */
static volatile int endProcess = FALSE;

/* Manejador de senales */
void manejador(int sig) {
    if (DEBUG) printf("process > manejador > signal = %d\n", sig);
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

int process_endProcess(){
    return endProcess;
}

/* Functions */

int process_parse_request(http_req *data, char *buf, int len) {
    int err, i;
    size_t prevbuflen = 0;

    sleep(3);

    /* Numero de */
    data->numHeaders = MAX_HEADERS;
    /* picohttpparser */
    err = phr_parse_request(buf, len, &(data->method), (size_t *)&(data->method_len), &(data->path),
                            (size_t *)&(data->path_len), &(data->version), (data->headers),
                            (size_t *)&(data->numHeaders), prevbuflen);

    data->total_len = err;
    if (DEBUG) {
        printf("\nprocess > process_parse_request:\n");

        /* Codigo de impresion de phr_header obtenido del
         * repositorio de github de picohttpparser */
        printf("\tRequest is %d bytes long\n", err);
        printf("\tMethod is %.*s\n", data->method_len, data->method);
        printf("\tPath is %.*s\n", data->path_len, data->path);
        printf("\tHTTP version is 1.%d\n", data->version);
        printf("\tHeaders:\n");
        for (i = 0; i != data->numHeaders; ++i) {
            printf("\t\t%.*s: %.*s\n", (int)data->headers[i].name_len, data->headers[i].name,
                   (int)data->headers[i].value_len, data->headers[i].value);
        }
    }

    if (err < 0) return err;

    if (sizeof(buf) == len) return -3;

    return 0;
}

/* Main processing function */
void process_request(int connfd) {
    char buf[MAX_BUF], auxstr[MAX_STR], *mstr;
    int n, err, i, keep_alive, verbo;
    http_req data;

    if (connfd < 0) return;

    do {
        /* Inicia el buffer a '\0' */
        if (!endProcess) {
            bzero(buf, MAX_BUF);
            /* Se lee la peticion del buffer */
            n = 0;
            while ((n += read(connfd, buf, sizeof(buf))) == -1 && errno == EINTR && !endProcess)
                ;

            if (n <= 0) {
                if (DEBUG && !endProcess)
                    printf("\nprocess > process_request > buffer read error %d\n", n);
                close(connfd);
                return;
            }
        }
        if (DEBUG && !endProcess)
            printf("\nprocess > process_request > buffer:\n##----##\n%s##----##\n", buf);

        /* Parse http request */
        if (!endProcess) {
            err = process_parse_request(&data, buf, n);
        }

        /*Error parsing*/
        if (err < 0) {
            if (DEBUG && !endProcess) printf("process > process_request error = %d\n", err);
            close(connfd);
            return;
        }

        /* Keep alive */
        for (i = 0, keep_alive = FALSE; i < data.numHeaders && !keep_alive && !endProcess; i++) {
            sprintf(auxstr, "%.*s", (int)data.headers[i].value_len, data.headers[i].value);
            if (!strcmp(auxstr, "Connection: keep-alive")) keep_alive = TRUE;
        }
        if (DEBUG && !endProcess) printf("keep-alive = %d\n", keep_alive);

        /* ACTUAL PROCESS REQUEST */
        if (!endProcess) {
            sprintf(auxstr, "%.*s", data.method_len, data.method);
            verbo = get_method(auxstr);
        }
        if (!endProcess) {
            switch (verbo) {
                case METHOD_GET:
                    mstr = GET;
                    err = process_get(&data, connfd);
                    break;
                case METHOD_POST:
                    mstr = POST;
                    err = process_post(&data, connfd);
                    break;
                case METHOD_OPTIONS:
                    mstr = OPTIONS;
                    err = process_options(&data, connfd);
                    break;

                default:
                    /* Verbo no soportado */
                    n = sprintf(buf, NOT_SUPPORTED_VERB);
                    write(connfd, buf, sizeof(char) * n);

                    if (DEBUG) printf("\nprocess_request > verbo no soportado %s\n", auxstr);
                    /* Cierre del descriptor de fichero */
                    close(connfd);
                    return;
            }

            if (DEBUG) printf("process_request > process_%s() = %d\n", auxstr, err);
        }
        /* end processing */

        /* Response */
        if (!endProcess) {
            n = sprintf(buf, "%s -> %s\r\r\n", DEFAULT_RESPONSE, mstr);
            write(connfd, buf, sizeof(char) * n);
        }
        /* End iteration */

    } while (keep_alive && !endProcess);
    /* Se cierra el descriptor de fichero de conexion */
    close(connfd);
    if (DEBUG && !endProcess) printf("process_request > conexion file descriptor closed\n");

    return;
}

/* Verbos soportados */
int get_method(char *mstr) {
    if (!strcmp(mstr, GET)) return METHOD_GET;
    if (!strcmp(mstr, POST)) return METHOD_POST;
    if (!strcmp(mstr, OPTIONS)) return METHOD_OPTIONS;

    return 0;
}

/* Funciones de los verbos soportados */
/* GET */
/*obtiene un recurso del servidor*/
int process_get(http_req *data, int connfd) {
    char buf_response[MAX_BUF], buf_date[MAX_BUF], buf_response2[MAX_BUF];
    char date[MAX_STR], last_date[MAX_STR];
    int err;

    if(!data || connfd<0) {
        printf("process > process_get > error en argumentos\n");
        close(connfd);
        return -1;
    }

    /*mostrar version HTTP y decir que procesa opciones*/
    sprintf(buf_response, "HTTP/1.%d 200 OK\r\nAllow: %s, %s, %s\r\n", data->version, GET, POST, OPTIONS);
    
    /*calcular fecha, aniadirla y poner nombre del servidor y la longitud del contenido*/
    get_date(&date);
    sprintf(buf_response2, "Date: %s\r\nServer: %s\r\nContent-Length: 0\r\n\r\n", date, data->headers->name);
    
    /*unir ambas partes de la respuesta*/
    strcat(buf_response, buf_response2);


    close(connfd);
    return 0;
}

/* POST */
/*para enviar una entidad a un recurso especificado, provocando cambios en el estado o en el servidor*/
int process_post(http_req *data, int connfd) {
    char buf_response[MAX_BUF], buf_date[MAX_BUF], buf_response2[MAX_BUF];
    char date[MAX_STR];
    int err;
    FILE *salida;

    if(!data || connfd<0) {
        printf("process > process_options > error en argumentos\n");
        close(connfd);
        return -1;
    }

    /*identificar el tipo de script*/
    if((strstr(data->path, ".py")) {
        sprintf(buf_response, "Python path : %s", data->path);
    } else if (strstr(data->path, ".php") {
        sprintf(buf_response, "Php path : %s", data->path);
    } else {
        printf("process > process_post > script incorrecto\n");

        close(connfd);
        return 0;
    }

    /*si tiene parametros la peticion*/
    if(!(strstr(data->path, "?")) {/*si tiene "?" significa que hay parametros*/
        
    } else { /*si no tiene parametros la peticion*/

    }
    

    close(connfd);
    return 0;
}

/* OPTIONS */
/*para describir las opciones de comunicacion del recurso destino*/
int process_options(http_req *data, int connfd) {
    char buf_response[MAX_BUF], buf_date[MAX_BUF], buf_response2[MAX_BUF];
    char date[MAX_STR];
    int err;

    if(!data || connfd<0) {
        printf("process > process_options > error en argumentos\n");
        close(connfd);
        return -1;
    }

    /*mostrar version HTTP y decir que procesa opciones*/
    sprintf(buf_response, "HTTP/1.%d 200 OK\r\nAllow: %s, %s, %s\r\n", data->version, GET, POST, OPTIONS);
    
    /*calcular fecha, aniadirla y poner nombre del servidor y la longitud del contenido*/
    get_date(&date);
    sprintf(buf_response2, "Date: %s\r\nServer: %s\r\nContent-Length: 0\r\n\r\n", date, data->headers->name);
    
    /*unir ambas partes de la respuesta*/
    strcat(buf_response, buf_response2);

    /*enviar el file descriptor del cliente a dicho cliente, escribiendo directamente en el descriptor de fichero*/
    err = write(connfd, buf_response, sizeof(buf_response));
    if(err < 0) {
        printf("process > process_options > no se puede responder al cliente\n");
        return -1;
    }

    close(connfd);

    return 0;
}
