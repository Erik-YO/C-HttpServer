

#include "hilos.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "types.h"

/* El nombre del semaforo */
#define LOCK_NAME "/hilos_lock_name"
#define COUNTER_NAME "/hilos_counter_name"

/* Estructura de gestion de hilos */
struct _GestorHilos {
    /*semaphore Lock para:*/
    sem_t* lock;
    /*hilo[] Thread array*/
    pthread_t** hilos;
    /*bool[] Is active thread*/
    short* taken;
    /*int Active threads*/
    sem_t* available;

    /*int Max threads*/
    int max;
};

/*Estructura intermedia para el lanzamiento de hilos*/
typedef struct _HiloLauncher {
    GestorHilos* gestor;
    funcionHilo func;
    void* arg;
    /*Indice del hilo en el array hilos del gestor*/
    int hilo;
} HiloLauncher;

/*Solo se permite tener un Gestor de Hilos activo*/
GestorHilos* globalGestor = NULL;
/*Para el timeout*/
int alarmed = 0;

void hilo_freeGestor(GestorHilos* gh);
void* _postLaunchHilo(void* launcher);

/*Funciones de gestion*/

/*
Create
Solo crea uno si no ha sido creado. Si ya habia uno creado se retorna
ese sin importar que el numero maxHilos no coincida
*/
GestorHilos* hilo_getGestor(int maxHilos) {
    GestorHilos* gh = NULL;
    int i;

    if (globalGestor != NULL) {
        return globalGestor;
    }

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
    for (i = 0; i < maxHilos; i++) {
        gh->hilos[i] = (pthread_t*)malloc(sizeof(pthread_t));
        if (!gh->hilos[i]) {
            while (i > 0) {
                i--;
                free(gh->hilos);
            }
            free(gh->hilos);
            free(gh);
            return NULL;
        }
    }

    /* Array de estado de los hilos */
    gh->taken = (short*)malloc(sizeof(short) * maxHilos);
    if (!gh->taken) {
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    /*
    O_CREAT especifica crear el semaforo, O_EXCL implica que sea el unico
    semaforo con ese nombre // S_IRUSR especifica que puede ser leido por el
    usuario y S_IWUSR que puede ser escrito
    */
    gh->lock = sem_open(LOCK_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
    if (gh->lock == SEM_FAILED) {
        free(gh->taken);
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    gh->available = sem_open(COUNTER_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, maxHilos);
    if (gh->lock == SEM_FAILED) {
        sem_close(gh->lock);
        sem_unlink(LOCK_NAME);

        free(gh->taken);
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    /*Al principio no hay ninguno ocupado*/
    for (i = 0; i < maxHilos; i++) {
        gh->taken[i] = FALSE;
    }

    gh->max = maxHilos;

    globalGestor = gh;

    return gh;
}

/*Close hilos*/
int hilo_closeHilos(GestorHilos* gh) {
    int i, t;
    if (!gh) return 1;

    for (i = 0; i < gh->max; i++) {
        sem_wait(gh->lock);
        t = gh->taken[i];
        sem_post(gh->lock);

        if (t) {
            /*Al cancelar el hilo se llama a las funciones introducidas por pthread_cleanup_push*/
            pthread_cancel(*(gh->hilos[i]));
        }
        gh->taken[i] = 0;
    }

    return 0;
}

/*Free gestor*/
void hilo_freeGestor(GestorHilos* gh) {
    int i;

    if (!gh) return;

    if (gh->lock) {
        sem_close(gh->lock);
        sem_unlink(LOCK_NAME);
    }

    if (gh->available) {
        sem_close(gh->available);
        sem_unlink(COUNTER_NAME);
    }

    if (gh->taken) {
        free(gh->taken);
    }

    for (i = 0; i < gh->max; i++) {
        if (gh->hilos[i]) {
            free(gh->hilos[i]);
        }
    }

    if (gh->hilos) {
        free(gh->hilos);
    }

    free(gh);

    return;
}

/*Destroy*/
int hilo_destroyGestor(GestorHilos* gh) {
    int i;

    if (!gh) return 0;

    sem_getvalue(gh->available, &i);
    if (i != gh->max) {
        return 1;
    }

    hilo_forceDestroyGestor(gh);
    return 0;
}

/*Force destroy*/
void hilo_forceDestroyGestor(GestorHilos* gh) {
    if (!gh) return;

    hilo_closeHilos(gh);

    hilo_freeGestor(gh);

    globalGestor = NULL;

    return;
}

/*Funciones de hilos*/

/*Launch hilo*/
int hilo_launch(GestorHilos* gh, funcionHilo fh, void* arg) {
    HiloLauncher launcher;
    int error, i, notTaken;

    launcher.gestor = gh;
    launcher.func = fh;
    launcher.arg = arg;

    sem_wait(gh->available);
    for (i = 0; i < gh->max; i++) {
        if (!(gh->taken[i])) {
            notTaken = i;
            i = gh->max;
        }
    }
    launcher.hilo = notTaken;

    /*Se marca el hilo como ocupado*/
    sem_wait(gh->lock);
    gh->taken[notTaken] = 1;
    sem_post(gh->lock);

    /*Crear hilo*/
    error = pthread_create(gh->hilos[i], NULL, _postLaunchHilo, (void*)&launcher);
    if (error) {
        sem_wait(gh->lock);
        gh->taken[i] = 0;
        sem_post(gh->lock);
        return 1;
    }

    return 0;
}

/*Launch hilo con timeout (en segundos) si no se consigue espacio para otro hilo en ese
 * tiempo*/
int hilo_lauchTimeOut(GestorHilos* gh, double timeout, funcionHilo fh, void* arg) {
    HiloLauncher launcher;
    int error, i, notTaken, sec, nsec;
    struct timespec time;

    /*Se configura el tiempo para que termine el timeout tras <timeout> segundos*/
    clock_gettime(CLOCK_REALTIME, &time);

    sec = (int)timeout;
    time.tv_sec += sec;
    time.tv_nsec += (int)((timeout - sec) * 1000000000);

    /*Si retorna un numero diferente de 0 es que ha habido algun error
    (como que haya pasado el tiempo de timeout sin que se haya liberado
    ningun espacio de hilo)*/
    error = sem_timedwait(gh->available, &time);
    if (error != 0) {
        return 1;
    }

    /*Preparamos la estructura para pasar a _postLaunchHilo*/
    launcher.gestor = gh;
    launcher.func = fh;
    launcher.arg = arg;

    for (i = 0; i < gh->max; i++) {
        if (!(gh->taken[i])) {
            notTaken = i;
            i = gh->max;
        }
    }
    launcher.hilo = notTaken;

    /*Se marca el hilo como ocupado*/
    sem_wait(gh->lock);
    gh->taken[notTaken] = 1;
    sem_post(gh->lock);

    /*Crear hilo*/
    error = pthread_create(gh->hilos[i], NULL, _postLaunchHilo, (void*)&launcher);
    if (error) {
        sem_wait(gh->lock);
        gh->taken[i] = 0;
        sem_post(gh->lock);
        return 1;
    }

    return 0;
}

/*Get active hilos*/
int hilo_getActive(GestorHilos* gh) {
    int i;
    if (!gh) return -1;
    sem_getvalue(gh->available, &i);
    return gh->max - i;
}

/*Get max hilos*/
int hilo_getMax(GestorHilos* gh) { return gh->max; }

void _makeAvailable(void* arg) {
    int* pos = (int*)arg;
    *pos = 0;
}

/*Llama a la funcion y al terminar deja libre */
void* _postLaunchHilo(void* launcher) {
    HiloLauncher* hl = (HiloLauncher*)launcher;

    pthread_detach(pthread_self());

    pthread_cleanup_push(sem_post, hl->gestor->lock);
    pthread_cleanup_push(_makeAvailable, &(hl->gestor->taken[hl->hilo]));
    pthread_cleanup_push(sem_wait, hl->gestor->lock);
    pthread_cleanup_push(sem_post, hl->gestor->available);

    hl->func(hl->arg);

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);

    pthread_exit(NULL);

    return NULL;
}
