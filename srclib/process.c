

#include "process.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "errno.h"
#include "picohttpparser.h"
#include "types.h"
#define MAX_BUF 8000
#define MAX_STR 90
#define METHOD_STR_LEN 10
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
    char method[METHOD_STR_LEN];
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


char get_date();

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
    size_t prevbuflen = 0, numHead=MAX_HEADERS;
    const char* method;

    if(!data || !buf) return -4;
    
    /* picohttpparser */
    err = phr_parse_request(buf, len, &(method), (size_t *)&(data->method_len), &(data->path), (size_t *)&(data->path_len),
                            &(data->version), (data->headers), &(numHead), prevbuflen);

    sprintf(data->method, "%.*s", data->method_len, method);
    /* Numero de cabeceras maximo */
    data->numHeaders = (int) numHead;
    data->total_len = err;
    if (config_debug()) {
        fprintf(config_debug_file(), "\nprocess > process_parse_request:\n");

        /* Codigo de impresion de phr_header obtenido del
         * repositorio de github de picohttpparser */
        fprintf(config_debug_file(), "\tRequest is %d bytes long\n", err);
        fprintf(config_debug_file(), "\tMethod is %.*s\n", data->method_len, data->method);
        fprintf(config_debug_file(), "\tPath is %.*s\n", data->path_len, data->path);
        fprintf(config_debug_file(), "\tHTTP version is 1.%d\n", data->version);
        fprintf(config_debug_file(), "\tHeaders:\n");
        for (i = 0; i != data->numHeaders; ++i) {
            fprintf(config_debug_file(), "\t\t%.*s: %.*s\n", (int)data->headers[i].name_len, data->headers[i].name, (int)data->headers[i].value_len,
                   data->headers[i].value);
        }
    }

    if (err < 0) return err;

    if (sizeof(buf) == len) return -3;

    return 0;
}

/* Main processing function */
void process_request(int connfd) {
    char buf[MAX_BUF]="\0", auxstr[MAX_STR], *mstr;
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
                if (config_debug() && !endProcess) fprintf(config_debug_file(), "\nprocess > process_request > buffer read error %d\n", n);
                close(connfd);
                return;
            }
        }
        if (config_debug() && !endProcess)
            fprintf(config_debug_file(), "\nprocess > process_request > buffer:\n##----##\n%s##----##\n", buf);

        /* Parse http request */
        if (!endProcess) {
            err = process_parse_request(&data, buf, n);
            if (config_debug()) fprintf(config_debug_file(), "\nprocess_request > verbo -> len = %d, '%.*s'\n", data.method_len, data.method_len, data.method);
            
        }

        /*Error parsing*/
        if (err < 0) {
            if (config_debug() && !endProcess) fprintf(config_debug_file(), "process > process_request error = %d\n", err);
            close(connfd);
            return;
        }

        /* Keep alive */
        for (i = 0, keep_alive = FALSE; i < data.numHeaders && !keep_alive && !endProcess; i++) {
            sprintf(auxstr, "%.*s", (int)data.headers[i].value_len, data.headers[i].value);
            if (!strcmp(auxstr, "Connection: keep-alive\r\n")) keep_alive = TRUE;
        }
        if (config_debug() && !endProcess) fprintf(config_debug_file(), "keep-alive = %d\n", keep_alive);

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

                    if (config_debug()) fprintf(config_debug_file(), "\nprocess_request > verbo no soportado %s\n", auxstr);
                    /* Cierre del descriptor de fichero */
                    close(connfd);
                    return;
            }

            if (config_debug()) fprintf(config_debug_file(), "process_request > process_%s() = %d\n", auxstr, err);
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
    if (config_debug() && !endProcess) fprintf(config_debug_file(), "process_request > conexion file descriptor closed\n");

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

    if (!data || connfd < 0) {
        if(config_debug()) fprintf(config_debug_file(), "process > process_get > error en argumentos\n");
        close(connfd);
        return -1;
    }

    /*mostrar version HTTP y decir que procesa opciones*/
    sprintf(buf_response, "HTTP/1.%d 200 OK\r\nAllow: %s, %s, %s\r\n", data->version, GET, POST, OPTIONS);

    /*calcular fecha, aniadirla y poner nombre del servidor y la longitud del contenido*/
    strcpy(date, get_date());
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
    char date[MAX_STR], ruta_file_petition[MAX_BUF], script_ejecucion[MAX_STR];
    char *ruta, *argumentos_ruta;
    int err;
    FILE *salida;

    if (!data || connfd < 0) {
        if(config_debug()) fprintf(config_debug_file(), "process > process_options > error en argumentos\n");
        close(connfd);
        return -1;
    }

    /*1. se ejecuta el fichero
    2. se guardar lo ejecutado en otro fichero
    3. se lee lo guardado
    4. se manda lo guardado*/

    /*si no tiene parametros la peticion*/
    if (!strstr(data->path, "?")) { /*si tiene "?" significa que hay parametros*/
        /*ruta_file_petition = ruta servidor + raiz servidor + ruta de la peticion*/
        sprintf(ruta_file_petition, "%s", data->path);

    } else { /*si tiene parametros la peticion*/
        ruta = strtok(data->path, "?");
        strcpy(ruta_file_petition, ruta); 
        /*capturar los argumentos*/
        while( ruta != NULL) {
            argumentos_ruta = strtok(NULL, s);
        }

        /*ruta_file_petition = ruta servidor + raiz servidor + ruta de la peticion*/
        sprintf(ruta_file_petition, "%s %s", data->path, ruta_file_petition);
        
        /*Ejecucion del string con los argumentos anadidos*/
        /*identificar el tipo de script*/
        if ( strstr(data->path, ".py") ) {
            sprintf(script_ejecucion, "python3 %s", data->path);
        } else if ( strstr(data->path, ".php") ) {
            sprintf(script_ejecucion, "php %s", data->path);
        } else {
            if(config_debug()) fprintf(config_debug_file(), "process > process_post > script incorrecto\n");

            close(connfd);
            return -1;
        }

        salida = popen(script_ejecucion, "r");
        if(!salida) {
            if(config_debug()) fprintf(config_debug_file(), "process > process_post > ejecucion incorrecta script\n");
            close(connfd);
            return -1;
        }
    }

    pclose(salida);
    close(connfd);
    return 0;
}

/* OPTIONS */
/*para describir las opciones de comunicacion del recurso destino*/
int process_options(http_req *data, int connfd) {
    char buf_response[MAX_BUF], buf_date[MAX_BUF], buf_response2[MAX_BUF];
    char date[MAX_STR];
    int err;

    if (!data || connfd < 0) {
        if(config_debug()) fprintf(config_debug_file(), "process > process_options > error en argumentos\n");
        close(connfd);
        return -1;
    }

    /*mostrar version HTTP y decir que procesa opciones*/
    sprintf(buf_response, "HTTP/1.%d 200 OK\r\nAllow: %s, %s, %s\r\n", data->version, GET, POST, OPTIONS);

    /*calcular fecha, aniadirla y poner nombre del servidor y la longitud del contenido*/
    strcpy(date, get_date());
    sprintf(buf_response2, "Date: %s\r\nServer: %s\r\nContent-Length: 0\r\n\r\n", date, config_server_signature());

    /*unir ambas partes de la respuesta*/
    strcat(buf_response, buf_response2);

    /*enviar el file descriptor del cliente a dicho cliente, escribiendo directamente en el descriptor de fichero*/
    err = write(connfd, buf_response, strlen(buf_response));
    if (err < 0) {
        if(config_debug()) fprintf(config_debug_file(), "process > process_options > no se puede responder al cliente\n");
        return -1;
    }

    close(connfd);

    return 0;
}



/* Privadas */
char get_date(){
    time_t t;
    struct tm *time_now;
    char date[MAX_STR];

    t=time(NULL);
    time_now=localtime(&t);

    /*construir el string de la fecha*/
    strftime(date, MAX_STR, "%a %d-%b-%Y %H:%M:%S GMT %z", time_now);
    
    return date;
}

