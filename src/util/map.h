//
// DEFINICIÓN DEL TIPO DE DATOS QUE GESTIONA UN MAPA GENÉRICO ITERABLE.
// NO PUEDE MODIFICAR ESTE FICHERO.

/*
 * Mapa: Colección de entradas -> [clave y valor]
 *
 * Además del acceso por clave, permite iterar por todas las entradas del
 * mapa a partir de la posición especificada.
 *
 * Almacena referencias (no copias) de cada clave y valor.
 *
 * Al crear el mapa, se indica indirectamente el tipo de la clave
 * especificando la función que se usará para comparar claves.
 *
 * Al destruir un map, su implementación invoca por cada entrada
 * existente en el mismo la función de liberación especificada como parámetro
 * para permitir que el usuario de esta colección pueda liberar,
 * si lo considera oportuno, la información asociada a cada clave y valor.
 *
 * Al eliminar una entrada, también se invoca la función especificada
 * como parámetro, si no es NULL, para esa entrada.
 */

#ifndef _MAP_H
#define _MAP_H      1

/* TIPOS DE DATOS REQUERIDOS POR EL API */

// Tipos opacos para ocultar la implementación
typedef struct map map; // el mapa
typedef struct map_iter map_iter; // un iterador
typedef struct map_position map_position; // una posición en el mapa

// Tipo de datos para una función que accede a una entrada para liberarla.
// Usado por map_destroy y map_remove_entry.
typedef void (*func_entry_release_map_t) (void *key, void *value);

// Tipo de datos para una función que accede a una entrada para visitarla.
// Usado por map_visit.
typedef void (*func_entry_map_t) (void *key, void *value, void *datum);

// Tipo de datos para una función que compara claves.
// Indica de forma indirecta cuál es el tipo de la clave.
// Debe devolver verdadero si son iguales.
typedef int (*func_cmp_keys_t) (const void *k1, const void *k2);

// funciones de comparación predefinidas para varios tipos de claves

// para strings
int key_string(const void *k1, const void *k2);

// para int
int key_int(const void *k1, const void *k2);

/* API: COLECCIÓN DE FUNCIONES */

// Crea un mapa. Recibe como parámetros la función que se usará
// para comparar la claves (que determina indirectamente el tipo de la clave)
// y si usa mutex para asegurar exclusión mutua en sus operaciones.
// Devuelve una referencia a un mapa o NULL en caso de error.
// Si se usa en un entorno multithread, se asume que será utilizada antes
// de crearse los threads que trabajarán con el mapa creado.
map *map_create(func_cmp_keys_t cmp, int locking);

// Destruye el mapa especificado. Si tiene todavía entradas
// se invocará la función especificada como parámetro por cada una de ellas
// pasando como argumentos a la misma la clave y valor de la entrada.
// Si la aplicación no está interesada en ser notificada de las entradas
// existentes, debe especificar NULL en el parámetro de esta función.
// Devuelve 0 si OK y -1 si error.
// Si se usa en un entorno multithread, se asume que será usada después de que
// terminen los threads que han utilizado el mapa que se va a destruir.
int map_destroy(map *m, func_entry_release_map_t release_entry);

// Permite iterar sobre todas las entradas de un mapa;
// Se invocará la función especificada como parámetro por cada una de ellas
// pasando como argumentos a la misma la clave y valor de la entrada,
// así como el dato recibido como argumento.
// Devuelve 0 si OK y -1 si error.
int map_visit(const map *m, func_entry_map_t visit_entry, void *datum);

// Inserta en el mapa la entrada especificada por la clave y el valor.
// Almacena referencias (y no copias) de la clave y el valor.
// Devuelve 0 si OK y -1 si error (p.e. si duplicada).
int map_put(map *m, const void *key, void *value);

// Retorna el valor asociado a una clave.
// Dado que cualquier valor es válido, devuelve en el tercer parámetro
// si se ha producido un error: 0 si OK y -1 si error.
void * map_get(const map *m, const void *key, int *error);

// Elimina una entrada dada su clave. Si se recibe un valor distinto
// de NULL en el tercer parámetro, se invoca esa función como parte
// de la eliminación de la entrada.
// Devuelve 0 si OK y -1 si error.
int map_remove_entry(map *m, const void *key, func_entry_release_map_t release_entry);

// Devuelve cuántas entradas tiene un mapa.
int map_size(const map *m);

// Crea una variable para almacenar una posicíón dentro del mapa.
// Esa variable se especificará como parámetro de "map_iter_init"
// para indicar por qué posición comienza una iteración y se actualizará
// con el resultado de "map_iter_exit" para guardar dónde terminó el iterador
// y empezar la próxima iteración a partir de ese punto.
// Devuelve una posición o un número negativo en caso de error.
map_position * map_alloc_position(map *m);

// Libera los recursos asociados a una posición.
int map_free_position(map_position *p);

// Crea un iterador asociado al mapa que recorrerá todas las entradas del
// mapa comenzando por la posición especificada por "p".
// Obtiene el mutex del mapa, si es que se ha activado esa opción, para que
// la iteracion se realice en exclusión mutua;
// es necesario terminar llamando a "map_iter_exit" para liberar ese mutex.
map_iter *map_iter_init(map *m, map_position *p);

// Comprueba si el iterador ha llegado al final
int map_iter_has_next(map_iter *i);

// Avanza el iterador
int map_iter_next(map_iter *i);

// Obtiene clave y valor apuntados por el iterador;
// si se quiere solo la clave o el valor, especifique NULL en el otro campo
int map_iter_value(map_iter *i, const void **key, void **value);

// Destruye un iterador, liberando el mutex.
// Devuelve un descriptor asociado a la posición donde terminó el iterador
// para poder ser usado en el "map_iter_init" de una próxima iteración
// o NULL en caso de error.
map_position * map_iter_exit(map_iter *i);

#endif // _MAP_H
