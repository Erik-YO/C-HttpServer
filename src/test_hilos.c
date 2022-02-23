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
#include <hilos.h>

/*funcion prueba para ejecutar los hilos*/
void *f_prueba(void *arg) {
    const char *msg = arg;
    int i;

    for (i = 0; i < strlen(msg); i++) {
        printf(" %c ", msg[i]);
        fflush(stdout);
        sleep(1);
    }

    return NULL;
}


int main() {
    int numMaxHilos=7, err;
    GestorHilos *g=NULL;

    /*creacion de un gestor hilos*/
    g=hilo_getGestor(numMaxHilos);
    if(!g) {
        hilo_destroyGestor(g);
        printf("Error al crear gestor hilos. \n");
        g=NULL;
    } else {
        printf("OK: creacion de un gestor hilos inicio \n");
    }

    /*comprobar destruir gestor hilos*/
    err = hilo_destroyGestor(g);
    if(err != 0) {
        hilo_forceDestroyGestor(g);
        printf("Error al destruir gestor hilos. \n");
        g=NULL;
    }else {
        printf("OK: destruir gestor hilos \n");
    }

    /*creacion de un gestor hilos*/
    g=hilo_getGestor(9);
    if(!g) {
        hilo_destroyGestor(g);
        printf("Error al crear gestor hilos. \n");
        g=NULL;
    } else {
        printf("OK: creacion de un gestor hilos tras destruir otro \n");
    }


    /*comprobar numMaxHilos*/
    if(hilo_getMax(g)!=9) {
        hilo_destroyGestor(g);
        printf("Error al comprobar numMaxHilos. \n");
    } else {
        printf("OK: asignar num max hilos y se comprueba el num. \n");
    }

    /*probar funcion hilos*/
    err = hilo_launch(g, f_prueba, " Mensaje ");
    if(err != 0) {
        hilo_destroyGestor(g);
        printf("Error al probar funcion hilos. \n");
    } else {
        printf("OK: probar funcion hilos \n");
    }

    /*comprobar que se han activado hilos*/
    err = hilo_getActive(g);
    if(err < 1) {
        hilo_destroyGestor(g);
        printf("Error al probar funcion hilos por no haberse activado ningun hilo \n");
    } else {
        printf("OK: num hilos activos tras el launch \n");
    }

    

    return 0;

}
