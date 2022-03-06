

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

/*
 * Funciones de control
 */

/*
 * FUNCION: int config_initFromFile()
 * DESCRIPCION: Establece el valor de las variables de configuracion segun el fichero
 *              server.conf (Ejecutar al inicializar)
 * ARGS_OUT: int - devuelve 0 si se ha ejecutado correctamente y <0 en caso contrario
 */
int config_initFromFile();

/*
 * FUNCION: void config_close_debug_file()
 * DESCRIPCION: Cierra el fichero de escritura de mensajes de error y debug. Si
 *              no habia ninguno abierto o es una salida estandar no hace nada
 */
void config_close_debug_file();

/*
 * FUNCION: void config_printHelp(FILE* f)
 * ARGS_IN: FILE* - fichero de impresion (si es NULL imprime en la salida estandar 'stdout')
 * DESCRIPCION: Imprime informacion sobre como utilizar el modulo de configuracion (sus variables)
 */
void config_printHelp(FILE* f);

/*
 * Funciones 'GET'
 */

/*
 * FUNCION: char* config_server_root()
 * DESCRIPCION: Devuelve el valor de la variable server_root
 * ARGS_OUT: char* - ruta al directorio que contiene los ficheros y recursos del servidor Web,
 *                   relativa al directorio donde se encuentra el fichero de configuracion
 */
char* config_server_root();

/*
 * FUNCION: int config_max_clients()
 * DESCRIPCION: Devuelve el valor de la variable max_clients
 * ARGS_OUT: int - numero maximo de clientes que el servidor podra atender simultaneamente
 */
int config_max_clients();

/*
 * FUNCION: int config_listen_port()
 * DESCRIPCION: Devuelve el valor de la variable listen_port
 * ARGS_OUT: int - puerto en el que el servidor debe recibir las conexiones entrantes
 */
int config_listen_port();

/*
 * FUNCION: char* config_server_signature()
 * DESCRIPCION: Devuelve el valor de la variable server_signature
 * ARGS_OUT: char* - cadena que sera devuelta en cada cabecera 'ServerName' posterior
 */
char* config_server_signature();

/*
 * FUNCION: int config_max_threads()
 * DESCRIPCION: Devuelve el valor de la variable max_threads
 * ARGS_OUT: int - indica el numero de hilos que se creara para atender a las peticiones entrantes
 */
int config_max_threads();

/*
 * FUNCION: FILE* config_debug_file()
 * DESCRIPCION: Devuelve el valor de la variable debug_file
 * ARGS_OUT: FILE* - fichero en el que se escribiran los mensajes de error y debug
 */
FILE* config_debug_file();

/*
 * FUNCION: int config_debug()
 * DESCRIPCION: Devuelve el valor de la variable debug
 * ARGS_OUT: int - indica si esta activa la escritura de mensajes de debug (0=no, 1=si)
 */
int config_debug();

/*
 * Funciones 'GET' de los valores por defecto
 */

/*
 * FUNCION: char* config_default_server_root()
 * DESCRIPCION: Devuelve el valor por defecto de la variable server_root
 * ARGS_OUT: char* - ruta al directorio que contiene los ficheros y recursos del servidor Web,
 *                   relativa al directorio donde se encuentra el fichero de configuracion
 */
char* config_default_server_root();

/*
 * FUNCION: int config_default_max_clients()
 * DESCRIPCION: Devuelve el valor por defecto de la variable max_clients
 * ARGS_OUT: int - numero maximo de clientes que el servidor podra atender simultaneamente
 */
int config_default_max_clients();

/*
 * FUNCION: int config_default_listen_port()
 * DESCRIPCION: Devuelve el valor por defecto de la variable listen_port
 * ARGS_OUT: int - puerto en el que el servidor debe recibir las conexiones entrantes
 */
int config_default_listen_port();

/*
 * FUNCION: char* config_default_server_signature()
 * DESCRIPCION: Devuelve el valor por defecto de la variable server_signature
 * ARGS_OUT: char* - cadena que sera devuelta en cada cabecera 'ServerName' posterior
 */
char* config_default_server_signature();

/*
 * FUNCION: int config_default_max_threads()
 * DESCRIPCION: Devuelve el valor por defecto de la variable max_threads
 * ARGS_OUT: int - indica el numero de hilos que se creara para atender a las peticiones entrantes
 */
int config_default_max_threads();

/*
 * FUNCION: FILE* config_default_debug_file()
 * DESCRIPCION: Devuelve el valor por defecto de la variable debug_file
 * ARGS_OUT: FILE* - fichero en el que se escribiran los mensajes de error y debug
 */
FILE* config_default_debug_file();

/*
 * FUNCION: int config_default_debug()
 * DESCRIPCION: Devuelve el valor por defecto de la variable debug
 * ARGS_OUT: int - indica si esta activa la escritura de mensajes de debug (0=no, 1=si)
 */
int config_default_debug();

#endif
