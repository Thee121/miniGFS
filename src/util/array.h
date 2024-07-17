//
// DEFINICIÓN DEL TIPO DE DATOS QUE GESTIONA UN "APPEND-ONLY ARRAY"
// DISEÑADO ESPECÍFICAMENTE PARA LA PRÁCTICA.
// NO PUEDE MODIFICAR ESTE FICHERO.

/*
 * Array: Lista de entradas gestionada con un esquema FIFO. Solo permite
 * añadir entradas al final. Proporciona acceso por posición a sus
 * elementos.
 *
 * Almacena una referencia (no copia) de cada valor.
 *
 * No hay funciones destructivas porque la práctica no lo requiere.
 *
 */

#ifndef _ARRAY_H
#define _ARRAY_H      1

// Tipo opaco para ocultar la implementación
typedef struct array array;

// Tipo de datos para una función que accede a una entrada para visitarla.
// Usado por array_visit.
typedef void (*func_entry_array_t) (void *value, void *datum);

/* API: COLECCIÓN DE FUNCIONES */

// Crea un array de solo "append".
// Recibe como parámetro si usa mutex para asegurar exclusión
// mutua en sus operaciones.
// Devuelve una referencia a un array o NULL en caso de error.
array *array_create(int locking);

// Inserta al final del array un nuevo elemento.
// Almacena una referencia (y no copia) del valor.
// Devuelve posición en el array si OK y -1 si error.
int array_append(array *a, const void *elem);

// Obtiene, sin extraer, el elemento seleccionado.
// Dado que cualquier valor retornado es válido, devuelve en el
// segundo parámetro si se ha producido un error: 0 si OK y -1 si error.
void * array_get(const array *a, int pos, int *error);

// Devuelve el nº de elementos en el array, -1 si error.
int array_size(const array *a);

// Permite iterar sobre todas las entradas de un array.
// Se invocará la función especificada como parámetro por cada una de ellas
// pasando como argumentos a la misma el valor de la entrada y el dato
// recibido como argumento.
// Devuelve el número de entradas visitadas si OK y -1 si error.
int array_visit(const array *a, func_entry_array_t visit_entry, void *datum);

#endif // _ARRAY_H
