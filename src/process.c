





#include "process.h"

#include "types.h"
#include "picohttpparser.h"
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_BUF 8000
#define MAX_HEADERS 100







/* almacena toda la informacion de una peticion http */
typedef struct {

    /*Cabeceras*/
    struct phr_header headers[MAX_HEADERS];
    /*Numero real de cabeceras*/
    int numHeaders;
    /*Bytes len of request*/
    int len;
    /*Metodo: GET, POST, ...*/
    const char *method;
    /*Ruta*/
    const char *path;
    /* Http version = 1.<version> */
    int version;

}http_req;





int parse_request(http_req *data, char *buf, int len){
    int err;
    struct phr_header headers[MAX_HEADERS];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;

    err = phr_parse_request(buf, len, &(data->method), &method_len, &(data->path), &path_len,
                             &(data->version), &(data->headers), &(data->numHeaders), prevbuflen);

    if(err < 0) return err;

    return 0;
}






void process_request(int connfd){
    char buf[MAX_BUF], phrase[]="Peticion devuelta";
    int n, err;
    http_req data;

    if(connfd < 0) return;

    /* Inicia el buffer a '\0' */
    bzero(buf, MAX_BUF);
    n = read(connfd, buf, sizeof(buf));
    if(DEBUG) printf("\nprocess > process_request > buffer-> %s\n", buf);

    /*Parse http request*/
    err = parse_request(&data, buf, n);

    /*Error parsing*/
    if(err < 0){
        if(DEBUG) printf("process > process_request error = %d\n", err);
        close(connfd);
        return;
    }









    n = sprintf(buf, "%s\r\r\n", phrase);
    write(connfd, buf, sizeof(char)*n);

    close(connfd);

	return;
}
   
