// EJEMPLO DE USO DE LOS TIPOS DE DATOS "MAP" Y "ARRAY"
// SE GESTIONA UN MAPA TAL QUE CADA ENTRADA TIENE UN ARRAY
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "map.h"
#include "array.h"

typedef struct cuenta {
    char *titular;
    int edad;
    int saldo_ini;
    array *operaciones;
} cuenta;

typedef struct operacion {
    int cantidad;
    time_t fecha;
} operacion;

static void imprime_operacion(operacion *o) {
    printf("cantidad %d fecha %s", o->cantidad, asctime(localtime(&o->fecha)));
}
static void imprime_entrada_array(void *v, void *d) {
    operacion *o = v;
    printf("\t");
    if (d) printf("%s: ", (char *)d);
    imprime_operacion(o);
}
static void imprime_entrada_mapa(void *k, void *v, void *d) {
    // se puede acceder al titular tanto usando la clave como a través del valor
    cuenta *c = v;
    if (d) printf("%s: ", (char *)d);
    printf("titular %s edad %d saldo inicial %d\n", c->titular,
	c->edad, c->saldo_ini);
    // recorre todos los elementos del array 
    array_visit(c->operaciones, imprime_entrada_array, NULL);
}

int main(int argc, char *argv[]) {
    map *cuentas;
    int err = 0;
    cuenta *c;
    operacion *op;

    // vamos a usar un ejemplo con cierta similitud a la práctica
    // donde se utiliza un mapa tal que en el valor asociado a una entrada
    // del mismo, que tiene como clave un string, hay un array.

    // se trata de una colección de cuentas bancarias, tal que cada cuenta
    // tiene asociado el nombre del titular, su edad, el saldo inicial y un
    // array de operaciones, incluyendo cada operación la cantidad y la fecha;
    // para mantener toda la información de una cuenta agrupada,
    // se ha optado por incluir también la clave (titular) en la estructura
    // que se usa como valor

    // creamos el mapa de cuentas: como no lo vamos a usar desde múltiples
    // threads, 2º parámetro = 0 (no usa mutex).
    // EN LA PRÁCTICA CREAREMOS UN MAPA EN LA INICIALIZACIÓN DEL MASTER
    // PARA ALMACENAR LA INFORMACIÓN DE LOS FICHEROS.
    // EN ESE MAPA EL NOMBRE DEL TEMA ACTUARÁ DE CLAVE. NÓTESE QUE DICHO
    // MAPA SE ACCEDERÁ DESDE MÚLTIPLES THREADS
    printf("CREO UN MAPA (map_create)\n");
    cuentas = map_create(key_string, 0); // sin cerrojos; en la práctica sí

    // vamos a crear varias cuentas;
    // EN EL MASTER HAY QUE AÑADIR UNA NUEVA ENTRADA POR CADA FICHERO CREADO
    // ASOCIANDO UN ARRAY VACÍO CON EL FICHERO PARA GUARDAR LOS DESCRIPTORES
    // DE SUS BLOQUES.
    printf("Y AÑADO 4 ENTRADAS (map_put)\n");
    printf("TAL QUE CADA ENTRADA CONTIENE UN ARRAY (array_create)\n");
    cuenta *c1 = malloc(sizeof(cuenta));
    c1->titular="Juan"; c1->edad=11; c1->saldo_ini=1000;
    c1->operaciones=array_create(0); // sin cerrojos; en la práctica sí
    map_put(cuentas, c1->titular, c1);

    cuenta *c2 = malloc(sizeof(cuenta));
    c2->titular="Luis"; c2->edad=22; c2->saldo_ini=2000;
    c2->operaciones=array_create(0);
    map_put(cuentas, c2->titular, c2);

    cuenta *c3 = malloc(sizeof(cuenta));
    c3->titular="Sara"; c3->edad=33; c3->saldo_ini=3000;
    c3->operaciones=array_create(0);
    map_put(cuentas, c3->titular, c3);

    cuenta *c4 = malloc(sizeof(cuenta));
    c4->titular="Ana"; c4->edad=44; c4->saldo_ini=4000;
    c4->operaciones=array_create(0);
    map_put(cuentas, c4->titular, c4);

    printf("\nIMPRIMO EL TAMAÑO DEL MAPA (map_size)\n");
    // imprimimos el tamaño del mapa
    printf("Hay %d cuentas\n", map_size(cuentas));
    
    // vamos a acceder a alguna entrada del mapa e imprimir algún dato;
    // EN LA PRÁCTICA, ESTA ACCIÓN SE REQUIERE EN DISTINTAS OPERACIONES
    printf("\nACCESO A UNA ENTRADA DEL MAPA POR CLAVE (map_get)\n");
    c = map_get(cuentas, "Sara", &err);
    if (err == -1)
	printf("no debe salir\n");
    printf("Saldo inicial de la cuenta de Sara %d\n", c->saldo_ini);

    // para depurar imprimimos los valores recorriendo todo el mapa;
    // en la función "imprime_entrada_mapa" se incluye a su vez un
    // "array_visit" para imprimir el contenido del array (por ahora, vacío).
    // El último parámetro será recibido por la función especificada.
    // EN LA PRÁCTICA NOS VA A FACILITAR LA DEPURACIÓN AL PODER MOSTRAR
    // EL ESTADO DEL MAPA DE FICHEROS.
    printf("\nIMPRIMIR TODO EL MAPA (map_visit)\n");
    map_visit(cuentas, imprime_entrada_mapa, "Estado del mapa de cuentas");

    // ya existe, da error
    // EN LA PRÁCTICA NOS PERMITIRÁ DETECTAR QUE UN FICHERO YA EXISTE
    printf("\nINTENTO CREAR UNA ENTRADA REPETIDA (map_put)\n");
    if (map_put(cuentas, c1->titular, c1)<0)
	printf("error en map_put\n");

    // vamos a añadir varias operaciones a la cuenta de Juan
    // EN LA PRÁCTICA, SE REQUIERE EN LA OPERACIÓN DE ESCRITURA EN EL MASTER
    printf("\nAÑADO TRES REGISTROS AL ARRAY ASOCIADO A UNA ENTRADA DEL MAPA (array_append)\n");
    c = map_get(cuentas, "Juan", &err);
    operacion *op1 = malloc(sizeof(operacion));
    op1->cantidad=100; op1->fecha=time(NULL);
    array_append(c->operaciones, op1);
    operacion *op2 = malloc(sizeof(operacion));
    op2->cantidad=200; op2->fecha=time(NULL);
    array_append(c->operaciones, op2);
    operacion *op3 = malloc(sizeof(operacion));
    op3->cantidad=-150; op3->fecha=time(NULL);
    array_append(c->operaciones, op3);
    
    // vamos a acceder a la segunda operación (pos=1) de la cuenta de Juan;
    // EN LA PRÁCTICA SE USARÁ EN LA LECTURA EN EL MASTER.
    printf("\nACCESO A UNA ENTRADA DEL ARRAY POR POSICIÓN (array_get)\n");
    op = array_get(c->operaciones, 1, &err);
    if (err!=-1) {
        printf("Segunda operación de la cuenta de %s: ", c->titular);
	imprime_operacion(op);
    }

    // imprimimos el tamaño del array de operaciones asociada a Juan
    printf("\nIMPRIMO EL TAMAÑO DEL ARRAY (array_size)\n");
    printf("En la cuenta de %s hay %d operaciones\n",
		 c->titular, array_size(c->operaciones));

    // para depurar comprobamos de nuevo los valores;
    // recuerde que se imprimen también las operaciones porque hemos incluido un
    // array_visit dentro de imprime_entrada_mapa.
    // EN LA PRÁCTICA, INCLÚYALO EN TODOS LOS PUNTOS SIGNIFICATIVOS.
    printf("\nIMPRIMIR TODO EL MAPA (map_visit)\n");
    printf("QUE IMPRIME TAMBIÉN LOS ARRAYS NO VACÍOS (array_visit)\n");
    map_visit(cuentas, imprime_entrada_mapa, "Estado después de añadir ops a Juan");
    
    return 0;
}
