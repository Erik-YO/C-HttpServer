

#ifndef HILOS_H
#define HILOS_H

/* Define la estructura de gestion de hilos */
typedef struct _GestorHilos GestorHilos;

/* Define las funciones que pueden ejecutar los hilos */
typedef void* (*funcionHilo)(void*);

/*
 * Funciones del gestor de hilos
 */

/*
 * FUNCION: GestorHilos* hilo_getGestor(int maxHilos)
 * ARGS_IN: int - numero max de hilos que tendra
 * DESCRIPCION: Crea el gestor de hilos
 * ARGS_OUT: GestorHilos* - devuelve un gestor de hilos si fue correctamente creado
 *              y no habia otro creado previamente. Si ya habia uno creado, se retorna ese.
 *              Si se produce un error, retorna NULL
 */
GestorHilos* hilo_getGestor(int maxHilos);

/*
 * FUNCION: void hilo_destroyGestor(GestorHilos* gh)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Espera hasta que todos los hilos terminen su ejecucion y libera la memoria del gestor de hilos
 */
void hilo_destroyGestor(GestorHilos* gh);

/*Funciones de hilos*/

/*
 * FUNCION: int hilo_launch(GestorHilos* gh, funcionHilo fh, void* arg)
 * ARGS_IN: GestorHilos* - gestor de hilos
 *          funcionHilo - funcion que ejecutara el hilo
 *          void* - argumento de la funcion hilo
 * DESCRIPCION: Realiza el launch/lanzamiento de un hilo nuevo en el sistema multihilo
 * ARGS_OUT: int - devuelve 0 en caso de exito, pero si es 0>1 significa error
 */
int hilo_launch(GestorHilos* gh, funcionHilo fh, void* arg);

/*
 * FUNCION: int hilo_lauchTimeOut(GestorHilos* gh, double timeout, funcionHilo fh, void* arg);
 * ARGS_IN: GestorHilos* - gestor de hilos
 *          double - tiempo
 *          funcionHilo - funcion que ejecutara el hilo
 *          void* - argumento de la funcion hilo
 * DESCRIPCION: Realiza el launch/lanzamiento de un hilo nuevo en el sistema multihilo, pero
 *              esta vez con timeout si no se consigue espacio para otro hilo en ese tiempo
 * ARGS_OUT: int - devuelve 0 en caso de exito, 1 en caso de error
 */
int hilo_lauchTimeOut(GestorHilos* gh, double timeout, funcionHilo fh, void* arg);

/*
 * FUNCION: int hilo_getActive(GestorHilos* gh)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Obtiene el numero de hilos activos
 * ARGS_OUT: int - devuelve el numero de hilos activos o -1 en caso de error
 */
int hilo_getActive(GestorHilos* gh);

/*
 * FUNCION: int hilo_launch(GestorHilos* gh, funcionHilo fh, void* arg)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Obtiene el numero maximo de hilos que se pueden crear
 * ARGS_OUT: int - devuelve el numero maximo de hilos que se pueden crear
 */
int hilo_getMax(GestorHilos* gh);

/*
 * FUNCION: int hilos_waitUntilAvailable(GestorHilos* gh)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Espera hasta que haya al menos un hilo disponible
 * ARGS_OUT: int - 0 si la ejecucion ha sido correcta, < 0 en caso de error
 */
int hilos_waitUntilAvailable(GestorHilos* gh);

#endif
