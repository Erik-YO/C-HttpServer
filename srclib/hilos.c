

#include "hilos.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "types.h"

/* Para generar los nombres de los semaforos */
#define NAME_LEN 20
#define LOCK_NAME "/hilos_lock_"
int gestores = 0;

void manejador(int sig) { /* Manejar la recepcion de la señal SIGKILL */ }

/* Estructura de gestion de hilos */
struct _GestorHilos {
    /*semaphore name*/
    char sem_name[NAME_LEN];
    /*semaphore Lock para:*/
    sem_t* lock;
    /*hilo[] Thread array*/
    pthread_t* hilos;
    /*bool[] Is active thread*/
    short* taken;
    /*int Active threads*/
    int active;

    /*int Max threads*/
    int max;
};

/*Funciones de gestion*/

/*Create*/
GestorHilos* create_gestor(int maxHilos) {
    GestorHilos* gh = NULL;
    int i;

    if (maxHilos <= 0) {
        return NULL;
    }

    /* Estructura */
    gh = (GestorHilos*)malloc(sizeof(GestorHilos));
    if (!gh) return NULL;

    /* Array de hilos */
    gh->hilos = (pthread_t**)malloc(sizeof(pthread_t*) * maxHilos);
    if (!gh->hilos) {
        free(gh);
        return NULL;
    }

    /* Array de estado de los hilos */
    gh->taken = (short*)malloc(sizeof(short) * maxHilos);
    if (!gh->taken) {
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    /**/
    sprintf(gh->sem_name, "%s%d", LOCK_NAME, gestores);
    gh->sem_name[NAME_LEN - 1] = "\0";
    /*
    O_CREAT especifica crear el semaforo, O_EXCL implica que sea el unico
    semaforo con ese nombre // S_IRUSR especifica que puede ser leido por el
    usuario y S_IWUSR que puede ser escrito
    */
    gh->lock = sem_open(gh->sem_name, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
    if (gh->lock == SEM_FAILED) {
        free(gh->taken);
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    for (i = 0; i < maxHilos; i++) {
        gh->taken[i] = FALSE;
    }

    gh->active = 0;
    gh->max = maxHilos;

    gestores++;
    return gh;
}

/*Close hilos*/
int hilo_closeHilos(GestorHilos* gh);

/*Free gestor*/
void hilo_freeGestor(GestorHilos* gh) {
    if (!gh) return;
    if (gh->lock) {
        sem_close(gh->lock);
        sem_unlink(gh->sem_name);
    }
    if (gh->taken) {
        free(gh->taken);
    }
    if (gh->hilos) {
        free(gh->hilos);
    }
    free(gh);
    return;
}

/*Destroy*/
int hilo_destroyGestor(GestorHilos* gh) {
    int i, t;

    if (!gh) return;

    sem_wait(gh->lock);
    i = gh->active;
    sem_post(gh->lock);
    if (i != 0) {
        return 1;
    }

    hilo_forceDestroyGestor(gh);
    return 0;
}

/*Force destroy*/
void hilo_forceDestroyGestor(GestorHilos* gh) {
    int i, t;
    if (!gh) return;

    for (i = 0; i < gh->max; i++) {
        sem_wait(gh->lock);
        t = gh->taken[i];
        sem_post(gh->lock);

        if (t) {
            pthread_kill(gh->hilos[i], SIGKILL);
        }
    }

    hilo_freeGestor(gh);

    return;
}

/*Funciones de hilos*/

/*Launch hilo*/
int hilo_launch(GestorHilos* gh, funcionHilo fh, void* arg);

/*Launch hilo con timeout si no se consigue espacio para otro hilo en ese
 * tiempo*/
int hilo_lauchTimeOut(GestorHilos* gh, double timeout, funcionHilo fh,
                      void* arg);

/*Get active hilos*/
int hilo_getActive(GestorHilos* gh);

/*Get max hilos*/
int hilo_getMax(GestorHilos* gh);
