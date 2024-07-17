//
// IMPLEMENTACIÓN DEL TIPO DE DATOS QUE GESTIONA UN "APPEND-ONLY ARRAY"
// DISEÑADO ESPECÍFICAMENTE PARA LA PRÁCTICA.
// NO PUEDE MODIFICAR ESTE FICHERO.
// NO TIENE QUE REVISARLO: NO ES NECESARIO QUE CONOZCA LOS DETALLES DE LA
// IMPLEMENTACIÓN PARA USARLO. SOLO IMPORTA EL API.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "array.h"

// definición de tipos
struct entry {
    const void *elem;
};
#define MAGIC_ARR (('A' << 24) + ('R' << 16) + ('R' << 8) + 'A')

struct array {
    int magic;
    int nentries;
    int length;
    struct entry *collection;
    int locking;
    pthread_mutex_t mut;
    pthread_mutexattr_t mattr;
};

static int check_array(const array *a);
    
/* implementación de funciones de API */

// crea un array
array *array_create(int locking) {
    array *a = malloc(sizeof(array));
    if (!a) return NULL;
    a->magic=MAGIC_ARR;
    a->nentries=a->length=0;
    a->collection=NULL;
    a->locking=locking;
    if (locking) {
        pthread_mutexattr_init(&a->mattr);
        pthread_mutexattr_settype(&a->mattr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&a->mut, &a->mattr);
    }
    return a;
}
// accede a un elemento del array
void * array_get(const array *a, int pos, int *error){
    int err=0;
    void *v;
    if (check_array(a)==-1) err=-1;
    else {
        if (a->locking) pthread_mutex_lock((pthread_mutex_t *)&a->mut);
        if (pos < 0 || pos >= a->nentries)
            err = -1;
        else v = (void *)a->collection[pos].elem;
        if (a->locking) pthread_mutex_unlock((pthread_mutex_t *)&a->mut);
    }
    if (error) *error=err;
    return (err==-1?NULL:v);
}

// Inserta al final del array un nuevo elemento.
int array_append(array *a, const void *elem) {
    if (check_array(a) || !elem ) return -1;
    int res=0;
    if (a->locking) pthread_mutex_lock(&a->mut);
    a->nentries++;
    if (a->nentries>a->length) {
        a->collection=realloc(a->collection, a->nentries*sizeof(struct entry));
        if (!a->collection)
	    res=-1;
	else 
            a->length++;
    }
    if (res!=-1) {
            struct entry e = {elem};
            a->collection[a->nentries-1] =  e;
	    res=a->nentries-1;
    }
    if (a->locking) pthread_mutex_unlock(&a->mut);
    return res;
}
int array_size(const array *a){
    if (check_array(a)) return -1;
    return a->nentries;
}

// itera sobre un array
int array_visit(const array *a, func_entry_array_t visit_entry, void *datum) {
    if (check_array(a)==-1) return -1;
    int err = 0;
    if (visit_entry) {
        if (a->locking) pthread_mutex_lock((pthread_mutex_t *)&a->mut);
            for (int i=0; i<a->nentries; i++)
                visit_entry((void *)a->collection[i].elem, datum);
        if (a->locking) pthread_mutex_unlock((pthread_mutex_t *)&a->mut);
    }
    return err;
}
// implementación de función interna
static int check_array(const array *a){
    int res=0;
    if (a==NULL || a->magic!=MAGIC_ARR){
        res=-1; fprintf(stderr, "el array especificado no es válido\n");
    }
    return res;
}
