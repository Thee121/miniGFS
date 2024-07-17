#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/uio.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "master.h"
#include "server.h"
#include "mgfs.h"
#include "common.h"
#include "common_cln.h"

// TIPOS INTERNOS

// descriptor de un sistema de ficheros:
// tipo interno que almacena información de un sistema de ficheros
typedef struct mgfs_fs
{
    int socket;
    int def_blocksize;
    int def_rep_factor;
} mgfs_fs;

// descriptor de un fichero:
// tipo interno que almacena información de un fichero
typedef struct mgfs_file
{
    mgfs_fs *fs;
    char *fname;
    int blocksize;
    int rep_factor;
    int n_bloque_sig;
} mgfs_file;

/*
 * FASE 1: CREACIÓN DE FICHEROS
 */

// PASO 1: CONEXIÓN Y DESCONEXIÓN

// Establece una conexión con el sistema de ficheros especificado,
// fijando los valores por defecto para el tamaño de bloque y el
// factor de replicación para los ficheros creados en esta sesión.
// Devuelve el descriptor del s. ficheros si OK y NULL en caso de error.
mgfs_fs *mgfs_connect(const char *master_host, const char *master_port,
                      int def_blocksize, int def_rep_factor)
{
    mgfs_fs *fs = malloc(sizeof(mgfs_fs));
    if (fs == NULL)
    {
        perror("Error al asignar memoria para el descriptor de sistema de ficheros");
        return NULL;
    }
    else
    {
        fs->socket = create_socket_cln_by_name(master_host, master_port);
    }

    if (fs->socket < 0)
    {
        perror("Error al intentar crear el socket para el maestro");
        free(fs);
        return NULL;
    }
    else
    {
        fs->def_blocksize = def_blocksize;
        fs->def_rep_factor = def_rep_factor;

        return fs;
    }
}
// Cierra la conexión con ese sistema de ficheros.
// Devuelve 0 si OK y un valor negativo en caso de error.
int mgfs_disconnect(mgfs_fs *fs)
{
    if (fs == NULL)
    {
        perror("Error ya que no se puede desconectar de una conexión no existente");
        return -1;
    }
    else
    {
        close(fs->socket);
        free(fs);
        return 0;
    }
}
// Devuelve tamaño de bloque por defecto y un valor negativo en caso de error.
int mgfs_get_def_blocksize(const mgfs_fs *fs)
{
    if (fs == NULL)
    {
        perror("Error ya que no se puede devolver el tamaño del bloque de una conexión no existente");
        return -1;
    }
    else
    {
        int tamano = fs->def_blocksize;
        return tamano;
    }
}
// Devuelve factor de replicación por defecto y valor negativo en caso de error.
int mgfs_get_def_rep_factor(const mgfs_fs *fs)
{
    if (fs == NULL)
    {
        perror("Error ya que no se puede devolver el factor de replicación de una conexión no existente");
        return -1;
    }
    else
    {
        int tamano = fs->def_rep_factor;
        return tamano;
    }
}

// PASO 2: CREAR FICHERO

// Crea un fichero con los parámetros especificados.
// Si blocksize es 0, usa el valor por defecto.
// Si rep_factor es 0, usa el valor por defecto.
// Devuelve el descriptor del fichero si OK y NULL en caso de error.
mgfs_file *mgfs_create(const mgfs_fs *fs, const char *fname,
                       int blocksize, int rep_factor)
{
    if (fs == NULL)
    {
        perror("Error ya que no se puede crear un fichero en una conexión no existente");
        return NULL;
    }
    mgfs_file *file = malloc(sizeof(mgfs_file));
    if (file == NULL)
    {
        perror("Error al asignar memoria para crear el fichero");
        return NULL;
    }
    else
    {
        file->n_bloque_sig = 0;
        file->fs = (mgfs_fs *)fs;
        file->fname = strdup(fname);
    }

    if (blocksize == 0)
    {
        file->blocksize = fs->def_blocksize;
    }
    else
    {
        file->blocksize = blocksize;
    }

    if (rep_factor == 0)
    {
        file->rep_factor = fs->def_rep_factor;
    }
    else
    {
        file->rep_factor = rep_factor;
    }

    char codigo = 'C';

    int long_nombre = strlen(file->fname);
    int long_nombre_N = htonl(long_nombre);
    int tamanoBloque = file->blocksize;
    int repFactor = file->rep_factor;

    struct iovec iov[5];
    iov[0].iov_base = &codigo;
    iov[0].iov_len = sizeof(char);

    iov[1].iov_base = &long_nombre_N;
    iov[1].iov_len = sizeof(int);

    iov[2].iov_base = file->fname;
    iov[2].iov_len = long_nombre;

    iov[3].iov_base = &tamanoBloque;
    iov[3].iov_len = sizeof(int);

    iov[4].iov_base = &repFactor;
    iov[4].iov_len = sizeof(int);

    // Comprueba si se ha realizado correctamente la escritura para la creación del archivo
    if (writev(fs->socket, iov, 5) < 0)
    {
        perror("Error en la escritura para la creación del archivo");
        free(file);
        return NULL;
    }

    int resultado;
    if (recv(fs->socket, &resultado, sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        perror("Error al recibir datos por el socket correspondiente");
        free(file);
        return NULL;
    }
    else if (resultado == -1)
    {
        perror("Error al crear el fichero pedido");
        free(file);
        return NULL;
    }
    else
    {
        return file;
    }
}
// Cierra un fichero.
// Devuelve 0 si OK y un valor negativo si error.
int mgfs_close(mgfs_file *f)
{
    if (f == NULL)
    {
        perror("Error al intentar cerrar el fichero pedido");
        return -1;
    }
    else
    {
        free(f->fname);
        free(f);
        return 0;
    }
}
// Devuelve tamaño de bloque y un valor negativo en caso de error.
int mgfs_get_blocksize(const mgfs_file *f)
{
    if (f == NULL)
    {
        perror("Error al intentar obtener el tamaño del bloque el fichero pedido");
        return -1;
    }
    else
    {
        int blocksize = f->blocksize;
        return blocksize;
    }
}
// Devuelve factor de replicación y valor negativo en caso de error.
int mgfs_get_rep_factor(const mgfs_file *f)
{
    if (f == NULL)
    {
        perror("Error al intentar obtener el factor de replicación el fichero pedido");
        return -1;
    }
    else
    {
        int factor = f->rep_factor;
        return factor;
    }
}

// Devuelve el nº ficheros existentes y un valor negativo si error.
int _mgfs_nfiles(const mgfs_fs *fs)
{
    if (fs == NULL)
    {
        perror("Error al intentar obtener en número de ficheros existentes");
        return -1;
    }
    else
    {
        char codigo = 'N';
        write(fs->socket, &codigo, sizeof(char));

        int numeroArchivos;
        read(fs->socket, &numeroArchivos, sizeof(int));

        return numeroArchivos;
    }
}

// PASO 3: APERTURA DE FICHERO PARA LECTURA

// Abre un fichero para su lectura.
// Devuelve el descriptor del fichero si OK y NULL en caso de error.
mgfs_file *mgfs_open(const mgfs_fs *fs, const char *fname)
{
    if (fs == NULL)
    {
        perror("Error al abrrir un fichero para lectura, fichero no existe");
        return NULL;
    }
    mgfs_file *file = malloc(sizeof(mgfs_file));
    if (file == NULL)
    {
        perror("Error en malloc al intentar reservar memoria para el archivo al intentar abrirlo");
        return NULL;
    }

    file->fs = (mgfs_fs *)fs;
    file->fname = strdup(fname);

    char codigo = 'O';

    struct iovec iov[3];
    iov[0].iov_base = &codigo;
    iov[0].iov_len = sizeof(char);

    int nombreLongitud = strlen(file->fname);
    int nombreLongitudN = htonl(nombreLongitud);
    iov[1].iov_base = &nombreLongitudN;
    iov[1].iov_len = sizeof(int);

    iov[2].iov_base = file->fname;
    iov[2].iov_len = nombreLongitud;

    if (writev(fs->socket, iov, 3) < 0)
    {
        perror("Error en la escritura para la apertura del archivo");
        free(file);
        return NULL;
    }

    int resultadoAbrir;
    if (recv(fs->socket, &resultadoAbrir, sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        perror("error en recv");
        free(file);
        return NULL;
    }

    else if (resultadoAbrir == -1)
    {
        perror("Error al intentar abrir el fichero pedido");
        free(file);
        return NULL;
    }

    else if (recv(fs->socket, &file->blocksize, sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        perror("Error, no se ha podido recibir el tamaño de bloque");
        free(file);
        return NULL;
    }

    if (recv(fs->socket, &file->rep_factor, sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        perror("Error, no se ha podido recibir el tamaño de factor de replicacion");
        free(file);
        return NULL;
    }
    else
    {
        return file;
    }
}

/*
 * FASE 2: ALTA DE LOS SERVIDORES
 */

// Operación interna para test; no para uso de las aplicaciones.
// Obtiene la información de localización (ip y puerto en formato de red)
// de un servidor.
// Devuelve 0 si OK y un valor negativo si error.
int _mgfs_serv_info(const mgfs_fs *fs, int n_server, unsigned int *ip,
                    unsigned short *port)
{
    if (fs == NULL)
    {
        perror("Error al asignar memoria para dar de alta los servicios");
        return -1;
    }

    char codigo = 'I';

    struct iovec iov[2];
    iov[0].iov_base = &codigo;
    iov[0].iov_len = sizeof(char);

    int n_server_net = htonl(n_server);
    iov[1].iov_base = &n_server_net;
    iov[1].iov_len = sizeof(int);

    if (writev(fs->socket, iov, 2) < 0)
    {
        perror("Error en la escritura al intentar dar de alta a los servidores");
        return -1;
    }

    int port_actual;
    if (recv(fs->socket, &port_actual, sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        perror("Error en recv al intentar comprobar la veracidad del puerto");
        return -1;
    }
    else if (ntohl(port_actual) == -1)
    {
        perror("Error al intentar leer el puerto. El servidor no existe.");
        return -1;
    }
    *port = (unsigned short)ntohl(port_actual);

    int ip_actual;
    if (recv(fs->socket, &ip_actual, sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        perror("Error en recv al intentar comprobar la veracidad de la ip");
        return -1;
    }
    else if (ntohl(ip_actual) == -1)
    {
        perror("Error al intentar leer la IP. El servidor no existe.");
        return -1;
    }

    *ip = (unsigned int)ntohl(ip_actual);
    return 0;
}

/*
 * FASE 3: ASIGNACIÓN DE SERVIDORES A BLOQUES.
 */

// Operación interna: será usada por write.
// Asigna servidores a las réplicas del siguiente bloque del fichero.
// Devuelve la información de localización (ip y puerto en formato de red)
// de cada una de ellas.
// Retorna 0 si OK y un valor negativo si error.
int _mgfs_alloc_next_block(const mgfs_file *file, unsigned int *ips, unsigned short *ports)
{
    return 0;
}

// Obtiene la información de localización (ip y puerto en formato de red)
// de los servidores asignados a las réplicas del bloque.
// Retorna 0 si OK y un valor negativo si error.
int _mgfs_get_block_allocation(const mgfs_file *file, int n_bloque,
                               unsigned int *ips, unsigned short *ports)
{
    return 0;
}

/*
 * FASE 4: ESCRITURA EN EL FICHERO.
 */

// Escritura en el fichero.
// Devuelve el tamaño escrito si OK y un valor negativo si error.
// Por restricciones de la práctica, "size" tiene que ser múltiplo
// del tamaño de bloque y el valor devuelto deber ser igual a "size".
int mgfs_write(mgfs_file *file, const void *buff, unsigned long size)
{
    if (size % mgfs_get_blocksize(file))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
