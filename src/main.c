
#include <time.h>

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "hilos.h"
#include <time.h>


/**/
int sem_timedwait(sem_t* __restrict__ __sem, const struct timespec* __restrict__ __abstime);


int test1(){

    struct timespec tm;
    sem_t sem;
    int i=0;

    sem_init( &sem, 0, 0);

    do {
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += 1;
        i++;
        printf("i=%d\n",i);
        if (i == 10) {
            sem_post(&sem);
        }

    } while ( sem_timedwait( &sem, &tm ) == -1 );

    printf("Semaphore acquired after %d timeouts\n", i);
    return 0;
}



void* test_func(void *arg){
    int i;

    for(i=0; i<5; i++){

        printf("Hilo %ld - i=%d\n", (long)pthread_self()%100, i);
        sleep(1);

    }

    return NULL;
}



int test_gestor(GestorHilos* g){
    int i, err;

    for(i= 0; i<5; i++){
        err = hilo_launch(g, test_func, NULL);
        if(err){
            printf("hilo_launch error -> i = %d, \terr = %d\n", i, err);
            return 1;
        }
    }

    return 0;
}




int main(){
    GestorHilos* g;
    int res;

    g = hilo_getGestor(2);

    if(!g){
        printf("Error !g\n");
        return 1;
    }


    res = test_gestor(g);
    while(hilo_getActive(g)!=0){
        sleep(1);
    }


    printf("Ejecutado test = %d\n\n", res);
    hilo_destroyGestor(g);

    return 0;
}