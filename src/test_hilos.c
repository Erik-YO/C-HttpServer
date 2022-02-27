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

/*funcion prueba para ejecutar los hilos*/
void *f_prueba(void *arg) {
    const char *msg = arg;
    int i;

    for (i = 0; i < strlen(msg); i++) {
        printf("%c ", msg[i]);
        fflush(stdout);
        sleep(1);
    }

    return NULL;
}

/*test1: hilo_getMax, hilo_launch, hilo_getActive(*/
int test1_hilos(GestorHilos *g) {
    int err;

    if(!g) return 1;

    /*comprobar numMaxHilos*/
    if(hilo_getMax(g)!=9) {
        printf("Error al comprobar numMaxHilos. \n");
        return 2;
    } else {
        printf("OK: asignar num max hilos y se comprueba el num. \n");
    }

    /*probar funcion launch hilos*/
    err = hilo_launch(g, f_prueba, "Mensaje");
    if(err != 0) {
        printf("Error al probar funcion hilos. \n");
        return 3;
    } else {
        printf("OK: probar funcion hilos \n");
    }

    /*probar funcion launch hilos*/
    err = hilo_lauchTimeOut(g, 3, f_prueba, "Mensaje_2");
    if(err != 0) {
        printf("Error al probar funcion hilos. \n");
        return 4;
    } else {
        printf("OK: probar funcion hilos \n");
    }
    sleep(1);
    /*comprobar que se han activado hilos*/
    err = hilo_getActive(g);
    if(err != 2) {
        printf("Error al probar funcion hilos por no haberse activado algun hilo \n");
        return 5;
    } else {
        printf("OK: num hilos activos tras el launch \n");
    }

    return 0;
}


/*test2: pruebas para cerrar hilos*/
int test2_hilos(GestorHilos *g) {
    int err;

    if(!g) return 1;

    /*comprobar cerrar hilos en caso sin nada*/
    err = hilo_closeHilos(NULL);
    if(err != 1) {
        printf("Error cerrar hilos en caso NULL \n");
        return 2;
    } else {
        printf("OK: cerrar hilos funciona bien en caso de error \n");
    }

    /*comprobar cerrar hilos en caso normal*/
    hilo_destroyGestor(g);
    g=hilo_getGestor(9);

    /*comprobar num hilos activos tras cerrar*/
    err = hilo_getActive(g);
    if(err != 0) {
        printf("Error num hilos activos tras cerrar hilos. Da %d\n", err);
        return 4;
    } else {
        printf("OK: num hilos activos correctos (1) tras cerrar hilos. \n");
    }

    /*comprobar num hilos max tras cerrar*/
    err = hilo_getMax(g);
    if(err != 9) { /*sigue siendo 9 porque lo definimos asi en el int main*/
        printf("Error num hilos max tras cerrar hilos. Da %d \n", err);
        return 5;
    } else {
        printf("OK: num hilos max correctos tras cerrar hilos. \n");
    }

    return 0;
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
        return 1;
    } else {
        printf("OK: creacion de un gestor hilos inicio \n");
    }

    /*comprobar destruir gestor hilos*/
    err = hilo_destroyGestor(g);
    if(err != 0) {
        hilo_destroyGestor(g);
        printf("Error al destruir gestor hilos. \n");
        g=NULL;
        return 3;
    }else {
        printf("OK: destruir gestor hilos \n");
    }

    /*creacion de un gestor hilos*/
    g=hilo_getGestor(9);
    if(!g) {
        hilo_destroyGestor(g);
        printf("Error al crear gestor hilos. \n");
        g=NULL;
        return 4;
    } else {
        printf("OK: creacion de un gestor hilos tras destruir otro \n");
    }


    printf("\n--------------------------\n > Empieza test 1: \n");
    err=test1_hilos(g);
    if(err) {
        hilo_destroyGestor(g);
        printf("Error %d en el test 1 \n", err);
        g=NULL;
        return 5;
    } else {
        printf("OK: test 1 todo bien \n");
    }

    hilo_destroyGestor(g);
    g=hilo_getGestor(9);

    printf("\n--------------------------\n > Empieza test 2: \n");
    err=test2_hilos(g);
    if(err) {
        hilo_destroyGestor(g);
        printf("Error %d en el test 2 \n", err);
        g=NULL;
        return 6;
    } else {
        printf("OK: test 2 todo bien \n");
    }

    hilo_forceDestroyGestor(g);

    return 0;

}
