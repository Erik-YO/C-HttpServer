

#include "hilos.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h> /*para conversion de int a puntero*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "types.h"

/* Estructura de gestion de hilos */
struct _GestorHilos {
    /*semaphore Lock para:*/
    sem_t lock;
    /*hilo[] Thread array*/
    pthread_t* hilos;
    /*bool[] Is active thread*/
    int* taken;
    /*int Active threads*/
    sem_t available;

    /*int Max threads*/
    int max;
    /*1 si no hay ningun hilo acupadoallAvailable*/
    sem_t allAvailable;
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

/* Private definitions */

/*
 * FUNCION: int sem_timedwait(sem_t* __restrict__ __sem, const struct timespec* __restrict__ __abstime)
 * ARGS_IN: sem_t* - semaforo al que se le quiere poner restriccion de tiempo
 *          const struct timespec* - puntero a una estructura que especifica el tiempo de bloqueo del semaforo en segundos y nanosegundos
 * DESCRIPCION: Es una funcion de la biblioteca de <semaphore.h>. Bloquea un semaforo concreto por una cantidad concreta de tiempo
 * ARGS_OUT: int - 0 en caso de exito, sino es error (como que haya pasado el tiempo de timeout sin que se
 *           haya liberado ningun espacio de hilo) con cualquier otro numero
 */
int sem_timedwait(sem_t* __restrict__ __sem, const struct timespec* __restrict__ __abstime);

/*
 * FUNCION: void hilo_freeGestor(GestorHilos* gh)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Liberar la memoria del gestor de hilos
 */
void hilo_freeGestor(GestorHilos* gh);

/*
 * FUNCION: void* _postLaunchHilo(void* launcher)
 * ARGS_IN: void* - launcher/lanzador de hilo
 * DESCRIPCION: Es la funcion hilo que se pasa junto a sus argumentos en la funcion pthread_create
 *              basicamente completa la estructura para la creacion del hilo
 */
void* _postLaunchHilo(void* launcher);

/*
 * FUNCION: void* _clean_hilo(void* arg)
 * ARGS_IN: void* - argumento para crear un HiloLauncher dentro de la funcion
 * DESCRIPCION: Limpia el hilo, liberando sus recursos
 */
void _clean_hilo(void* arg);

/*
 * FUNCION: void _post_available(GestorHilos* g)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Hace sem_post a los semaforos disponibles
 */
void _post_available(GestorHilos* g);

/*
 * FUNCION: void _wait_available(GestorHilos* g)
 * ARGS_IN: GestorHilos* - gestor de hilos
 * DESCRIPCION: Hace sem_wait a los semaforos disponibles
 */
void _wait_available(GestorHilos* g);

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
    gh = (GestorHilos*)calloc(1, sizeof(GestorHilos));
    if (!gh) return NULL;

    /* Array de hilos */
    gh->hilos = (pthread_t*)calloc(maxHilos, sizeof(*(gh->hilos)));
    if (!gh->hilos) {
        free(gh);
        return NULL;
    }
    /*for (i = 0; i < maxHilos; i++) {
        gh->hilos[i] = 0;
    }*/

    /* Array de estado de los hilos */
    gh->taken = (int*)calloc(maxHilos, sizeof(int));
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
    i = sem_init(&(gh->lock), 1, 1);
    if (i < 0) {
        free(gh->taken);
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    i = sem_init(&(gh->available), 1, maxHilos);
    if (i < 0) {
        sem_destroy(&(gh->lock));

        free(gh->taken);
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    i = sem_init(&(gh->allAvailable), 1, TRUE);
    if (i < 0) {
        sem_destroy(&(gh->lock));
        sem_destroy(&(gh->available));

        free(gh->taken);
        free(gh->hilos);
        free(gh);
        return NULL;
    }

    /*Al principio no hay ninguno ocupado*/
    for (i = 0; i < maxHilos; i++) {
        gh->taken[i] = FALSE;
        gh->hilos[i] = 0;
    }

    gh->max = maxHilos;

    globalGestor = gh;

    return gh;
}


/*Free gestor*/
void hilo_freeGestor(GestorHilos* gh) {
    if (!gh) return;

    sem_destroy(&(gh->lock));
    sem_destroy(&(gh->available));

    if (gh->taken) {
        free(gh->taken);
    }

    if (gh->hilos) {
        free(gh->hilos);
    }

    free(gh);
    globalGestor = NULL;

    return;
}

/*Destroy*/
void hilo_destroyGestor(GestorHilos* gh) {
    int ended = 0;
    if (!gh) return;

    /* Espera hasta que todos los hilos hayan terminado su ejecucion */
    while (ended < gh->max) {
        hilos_waitUntilAvailable(gh);
        _post_available(gh);
        ended++;
    }

    if(config_debug()) fprintf(config_debug_file(), "hilos > hilo_destroyGestor\n");
    hilo_freeGestor(gh);
    return;
}



/*Funciones de hilos*/

/*Launch hilo*/
int hilo_launch(GestorHilos* gh, funcionHilo fh, void* arg) {
    HiloLauncher* launcher = NULL;
    int error, i, notTaken = -1;

    if (!gh || !fh) {
        if(config_debug()) fprintf(config_debug_file(), "hilos > hilo_launch > gh or fh =NULL\n");
        return 1;
    }
    if(config_debug()) fprintf(config_debug_file(), "hilos > hilo_launch\n");

    launcher = (HiloLauncher*)calloc(1, sizeof(HiloLauncher));
    if (!launcher) return 2;

    launcher->gestor = gh;
    launcher->func = fh;
    launcher->arg = arg;
    if (!arg) launcher->arg = NULL;

    _wait_available(gh);
    sem_wait(&(gh->lock));
    for (i = 0; i < gh->max; i++) {
        if (!(gh->taken[i])) {
            notTaken = i;
            i = gh->max;
        }
    }
    if (notTaken < 0) return 3;
    launcher->hilo = notTaken;

    /*Se marca el hilo como ocupado*/
    gh->taken[notTaken] = 1;
    sem_post(&(gh->lock));

    /*Crear hilo*/
    error = pthread_create(&(gh->hilos[notTaken]), NULL, _postLaunchHilo, (void*)launcher);
    if (error) {
        sem_wait(&(gh->lock));
        gh->taken[notTaken] = 0;
        sem_post(&(gh->lock));
        return 4;
    }

    if(config_debug()) fprintf(config_debug_file(), "hilos > hilo_launch > hilo creado\n");

    return 0;
}

/*Launch hilo con timeout (en segundos) si no se consigue espacio para otro hilo en ese
 * tiempo*/
int hilo_lauchTimeOut(GestorHilos* gh, double timeout, funcionHilo fh, void* arg) {
    HiloLauncher* launcher = NULL;
    int error, i, notTaken = -1, sec;
    struct timespec time;

    /*Se configura el tiempo para que termine el timeout tras <timeout> segundos*/
    clock_gettime(CLOCK_REALTIME, &time);

    sec = (int)timeout;
    time.tv_sec += sec;
    time.tv_nsec += (int)((timeout - sec) * 1000000000);

    /*Si retorna un numero diferente de 0 es que ha habido algun error
    (como que haya pasado el tiempo de timeout sin que se haya liberado
    ningun espacio de hilo)*/
    error = sem_timedwait(&(gh->available), &time);
    if (error != 0) {
        return 1;
    }

    /*Preparamos la estructura para pasar a _postLaunchHilo*/
    launcher = (HiloLauncher*)calloc(1, sizeof(HiloLauncher));
    launcher->gestor = gh;
    launcher->func = fh;
    launcher->arg = arg;

    for (i = 0; i < gh->max; i++) {
        if (!(gh->taken[i])) {
            notTaken = i;
            i = gh->max;
        }
    }
    if (notTaken < 0) return 2;
    launcher->hilo = notTaken;

    /*Se marca el hilo como ocupado*/
    sem_wait(&(gh->lock));
    gh->taken[notTaken] = 1;
    sem_post(&(gh->lock));

    /*Crear hilo*/
    error = pthread_create(&(gh->hilos[i]), NULL, _postLaunchHilo, (void*)launcher);
    if (error) {
        sem_wait(&(gh->lock));
        gh->taken[i] = 0;
        sem_post(&(gh->lock));
        return 1;
    }

    return 0;
}

/*Get active hilos*/
int hilo_getActive(GestorHilos* gh) {
    int i;
    if (!gh) return -1;
    sem_wait(&(gh->lock));
    sem_getvalue(&(gh->available), &i);
    sem_post(&(gh->lock));
    return gh->max - i;
}

/*Get max hilos*/
int hilo_getMax(GestorHilos* gh) { return gh->max; }

/* Espera hasta que haya al menos un hilo disponible */
int hilos_waitUntilAvailable(GestorHilos* gh) {
    if (!gh) return -1;
    _wait_available(gh);
    _post_available(gh);
    return 0;
}

void _makeAvailable(void* arg) {
    int pos = (intptr_t)arg;
    if (!globalGestor) return;
    globalGestor->taken[pos] = FALSE;
}

void _clean_hilo(void* arg) {
    HiloLauncher* hl = (HiloLauncher*)arg;
    if (!globalGestor) return;

    sem_wait(&(globalGestor->lock));
    if (!globalGestor) return;
    globalGestor->taken[hl->hilo] = FALSE;
    globalGestor->hilos[hl->hilo] = 0;
    sem_post(&(globalGestor->lock));
    _post_available(globalGestor);

    pthread_exit(NULL);

    return;
}

/*Llama a la funcion y al terminar deja libre */
void* _postLaunchHilo(void* launcher) {
    HiloLauncher* launch = (HiloLauncher*)launcher;
    HiloLauncher hl;

    hl.gestor = launch->gestor;
    hl.func = launch->func;
    hl.hilo = launch->hilo;
    hl.arg = launch->arg;
    free(launcher);

    pthread_detach(pthread_self());

    pthread_cleanup_push(_clean_hilo, (void*)&hl);

    hl.func(hl.arg);

    pthread_cleanup_pop(1);

    return NULL;
}

void _post_available(GestorHilos* g) {
    int i;
    if (!g) return;

    sem_post(&(g->available));
    sem_getvalue(&(g->available), &i);
    if (i == g->max) {
        sem_getvalue(&(g->allAvailable), &i);
        if (i != TRUE) {
            sem_post(&(g->allAvailable));
        }
    }
    return;
}

void _wait_available(GestorHilos* g) {
    int i;
    if (!g) return;

    sem_wait(&(g->available));
    sem_getvalue(&(g->allAvailable), &i);
    if (i == TRUE) {
        sem_wait(&(g->allAvailable));
    }
    return;
}
