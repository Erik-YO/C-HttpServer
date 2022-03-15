

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
 * FUNCION: int hilo_closeHilos(GestorHilos* gh)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Cierra todos los hilos activos
 * ARGS_OUT: int - devuelve 1 en caso de exito, 0 en caso de error
 */
int hilo_closeHilos(GestorHilos* gh);

/*
 * FUNCION: int hilo_closeHilos(GestorHilos* gh)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Destruye el gestor de hilos, lberando la memoria de los hilos que controlaba este
 * ARGS_OUT: int - devuelve 0
 */
int hilo_destroyGestor(GestorHilos* gh);

/*
 * FUNCION: int hilo_closeHilos(GestorHilos* gh)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Fuerza la detencion de todos los hilos activos, liberando incluso el gestor de hilos
 */
void hilo_forceDestroyGestor(GestorHilos* gh);





/*Funciones de hilos*/


/*Launch hilo*/
int hilo_launch(GestorHilos* gh, funcionHilo fh, void* arg);

/*Launch hilo con timeout si no se consigue espacio para otro hilo en ese tiempo*/
int hilo_lauchTimeOut(GestorHilos* gh, double timeout, funcionHilo fh, void* arg);

/*Get active hilos*/
int hilo_getActive(GestorHilos* gh);

/*Get max hilos*/
int hilo_getMax(GestorHilos* gh);





#endif
