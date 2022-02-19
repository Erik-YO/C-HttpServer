
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

int main(){

    struct timespec tm;
    sem_t sem;
    int i=0;

    sem_init( &sem, 0, 0);

    do {
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += 1;
        i++;
        printf("i=%d\n",i);
        if (i==10) {
            sem_post(&sem);
        }

    } while ( sem_timedwait( &sem, &tm ) == -1 );

    printf("Semaphore acquired after %d timeouts\n", i);
    return 0;
}