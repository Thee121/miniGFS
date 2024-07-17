/*
 *
 * API PARA EL CLIENTE.
 * NO MODIFICAR.
 *
 */
#ifndef _MGFS_H
#define _MGFS_H        1

// ******** API para las aplicaciones ********

// OPERACIONES SOBRE UN SISTEMA DE FICHEROS

// descriptor de un sistema de ficheros:
// tipo interno que almacena información de un sistema de ficheros
typedef struct mgfs_fs mgfs_fs;

// Establece una conexión con el sistema de ficheros especificado,
// fijando los valores por defecto para el tamaño de bloque y el
// factor de replicación para los ficheros creados en esta sesión.
// Devuelve el descriptor del s. ficheros si OK y NULL en caso de error.
mgfs_fs * mgfs_connect(const char *master_host, const char *master_port,
	int def_blocksize, int def_rep_factor);

// Cierra la conexión con ese sistema de ficheros.
// Devuelve 0 si OK y un valor negativo en caso de error.
int mgfs_disconnect(mgfs_fs *fs);

// Devuelve tamaño de bloque por defecto y un valor negativo en caso de error.
int mgfs_get_def_blocksize(const mgfs_fs *fs);

// Devuelve factor de replicación por defecto y valor negativo en caso de error.
int mgfs_get_def_rep_factor(const mgfs_fs *fs);

// OPERACIONES SOBRE UN FICHERO

// descriptor de un fichero:
// tipo interno que almacena información de un fichero
typedef struct mgfs_file mgfs_file;

// Crea un fichero con los parámetros especificados.
// Si blocksize es 0, usa el valor por defecto.
// Si rep_factor es 0, usa el valor por defecto.
// Devuelve el descriptor del fichero si OK y NULL en caso de error.
mgfs_file * mgfs_create(const mgfs_fs *fs, const char *fname,
	int blocksize, int rep_factor);

// Abre un fichero para su lectura.
// Devuelve el descriptor del fichero si OK y NULL en caso de error.
mgfs_file * mgfs_open(const mgfs_fs *fs, const char *fname);

// Cierra un fichero.
// Devuelve 0 si OK y un valor negativo si error.
int mgfs_close(mgfs_file *f);

// Devuelve tamaño de bloque y un valor negativo en caso de error.
int mgfs_get_blocksize(const mgfs_file *f);

// Devuelve factor de replicación y valor negativo en caso de error.
int mgfs_get_rep_factor(const mgfs_file *f);
 
// Escritura en el fichero.
// Devuelve el tamaño escrito si OK y un valor negativo si error.
// Por restricciones de la práctica, "size" tiene que ser múltiplo
// del tamaño de bloque y el valor devuelto deber ser igual a "size".
int mgfs_write(mgfs_file *file, const void *buff, unsigned long size);

// OPERACIONES INTERNAS: NO PARA USO DE LAS APLICACIONES.
// FACILITAN LA DEPURACIÓN Y EL DESARROLLO INCREMENTAL.

// Obtiene la información de localización (ip y puerto en formato de red)
// de un servidor.
// Devuelve 0 si OK y un valor negativo si error.
int _mgfs_serv_info(const mgfs_fs *fs, int n_server, unsigned int *ip,
	unsigned short *port);

// Devuelve el nº ficheros existentes y un valor negativo si error.
int _mgfs_nfiles(const mgfs_fs *fs);

// Operación interna: será usada por write.
// Asigna servidores a las réplicas del siguiente bloque del fichero.
// Devuelve la información de localización (ip y puerto en formato de red)
// de cada una de ellas.
// Retorna 0 si OK y un valor negativo si error.
int _mgfs_alloc_next_block(const mgfs_file *file, unsigned int *ips, unsigned short *ports);

// Obtiene la información de localización (ip y puerto en formato de red)
// de los servidores asignados a las réplicas del bloque.
// Retorna 0 si OK y un valor negativo si error.
int _mgfs_get_block_allocation(const mgfs_file *file, int n_bloque,
        unsigned int *ips, unsigned short *ports);


#endif // _MGFS_H
