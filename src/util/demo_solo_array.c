// EJEMPLO DE USO DEL TIPO DE DATOS "ARRAY"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "array.h"

typedef struct punto {
    int x;
    int y;
    int z;
} punto;

static void imprime_entrada_array(void *v, void *d) {
    punto *p = v;
    if (d) printf("%s: ", (char *)d);
    printf("(%d,%d,%d)\n", p->x, p->y, p->z);
}

int main(int argc, char *argv[]) {
    array *puntos;
    int err = 0;
    punto *p;

    // Este ejemplo maneja un array de puntos.
    // Creamos el array de puntos: como no lo vamos a usar desde múltiples
    // threads, parámetro = 0 (no usa mutex).
    // EN LA PRÁCTICA CREAREMOS UN ARRAY EN LA INICIALIZACIÓN DEL MASTER
    // PARA ALMACENAR LA INFORMACIÓN DE LOS SERVIDORES DISPONIBLES.
    // NÓTESE QUE EN LA PRÁCTICA DICHO ARRAY SE ACCEDERÁ DESDE MÚLTIPLES THREADS
    printf("CREO UN ARRAY (array_create)\n");
    puntos = array_create(0); // sin cerrojos; en la práctica sí

    // vamos a crear varios puntos;
    // EN EL MASTER HAY QUE AÑADIR UNA NUEVA ENTRADA POR CADA SERVIDOR.
    printf("Y AÑADO 3 ENTRADAS (array_append)\n");
    punto *p1 = malloc(sizeof(punto));
    p1->x=1; p1->y=11; p1->z=111;
    array_append(puntos, p1);

    punto *p2 = malloc(sizeof(punto));
    p2->x=2; p2->y=22; p2->z=222;
    array_append(puntos, p2);

    punto *p3 = malloc(sizeof(punto));
    p3->x=3; p3->y=33; p3->z=333;
    array_append(puntos, p3);

    // vamos a acceder a la segunda operación (pos=2) del array.
    printf("\nACCESO A UNA ENTRADA DEL ARRAY POR POSICIÓN (array_get)\n");
    p = array_get(puntos, 1, &err);
    if (err!=-1) printf("(%d,%d,%d)\n", p->x, p->y, p->z);

    // imprimimos el tamaño del array de puntos
    printf("\nIMPRIMO EL TAMAÑO DEL ARRAY (array_size)\n");
    printf("Hay %d puntos\n\n", array_size(puntos));

    // para depurar imprimimos todas las entradas del array
    // EN LA PRÁCTICA, INCLÚYALO EN TODOS LOS PUNTOS SIGNIFICATIVOS.
    printf("IMPRIMIR TODO EL MAPA (array_visit)\n");
    array_visit(puntos, imprime_entrada_array, "Lista de puntos");
    
    return 0;
}
