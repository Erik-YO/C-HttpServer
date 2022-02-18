

#ifndef HILOS_H
#define HILOS_H

/* Define la estructura de gestion de hilos */
typedef struct _GestorHilos GestorHilos;


/* Define las funciones que pueden ejecutar los hilos */
typedef void* (*funcionHilo)(void*);



/*Funciones de gestion*/

/*Create*/
GestorHilos* hilo_createGestor(int maxHilos);

/*Close hilos*/
int hilo_closeHilos(GestorHilos* gh);

/*Destroy*/
int hilo_destroyGestor(GestorHilos* gh);

/*Force destroy*/
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
