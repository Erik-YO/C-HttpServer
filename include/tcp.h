

#ifndef TCP_H
#define TCP_H


/*
 * Funciones de TCP (Protocolo de Control de Transmisión o Transmission Control Protocol)
 */

/*
 * FUNCION: int tcp_listen()
 * DESCRIPCION: aplica la funcion bind() (registrar el socket en un puerto pasandole el anterior IP 
 *          y puerto que se ha pasado) y listen() (prepara la disposicion a recibir llamadas) en el socket creado en la funcion
 *  ARGS_OUT: int - resultado: el descriptor de fichero del socket donde se esta aplicando la funcion 
 *          si se ha ejecutado correctamente, -1 en caso de error en la funcion, -2 en caso de error en las opciones del socket
 * */
int tcp_listen();

/*
 * FUNCION: tcp_accept(int sockfd)
 * ARGS_IN: int - descriptor de fichero del socket
 * DESCRIPCION: registrar el socket en un puerto y obtener ese descriptor de fichero de conexion correspondiente
 * ARGS_OUT: int - resultado: el descriptor de fichero de conexion correspondiente al socket 
 *          si se ha ejecutado correctamente, -1 en caso de error 
 */
int tcp_accept(int sockfd);

/*
 * FUNCION: tcp_connect(char *ip)
 * ARGS_IN: char* - ip
 * DESCRIPCION: conectar el socket cliente con el socket del servidor
 * ARGS_OUT: int - resultado: el descriptor de fichero de socket
 *          si se ha ejecutado correctamente, -1 en caso de error 
 */
int tcp_connect(char *ip);

#endif
