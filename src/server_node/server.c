#include <netinet/in.h>
#include <sys/uio.h>

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "common_srv.h"
#include "common_cln.h"

#include <sys/stat.h>
#include <sys/mman.h>

typedef struct thread_info
{
    int socket;
    char *dir;
} thread_info;

void *servicio(void *arg)
{
    thread_info *thinf = arg;

    // si recv returnea numero<=0 , el cliente ha forzado la interrupcion de la conexión;
    // Implementado con MSG_WAITALL para no tener problemas de llegada de información
    while (1)
    {
        int longitud_fname_net;
        if (recv(thinf->socket, &longitud_fname_net, sizeof(int), MSG_WAITALL) != sizeof(int))
        {
            perror("Error en el método recv, la longitud de fname no es correcta");
            break;
        }

        int longitud_fname = ntohl(longitud_fname_net);
        char *fname = malloc(longitud_fname + 1);
        if (recv(thinf->socket, fname, longitud_fname, MSG_WAITALL) != longitud_fname)
        {
            perror("Error en el método recv, no se ha creado en memoria el tamaño correcto");
            free(fname);
            break;
        }
        else
        {
            fname[longitud_fname] = '\0';
        }

        int n_bloque_net;
        if (recv(thinf->socket, &n_bloque_net, sizeof(int), MSG_WAITALL) != sizeof(int))
        {
            perror("Error en el método recv, no se ha detectado el numero de bloque correcto");
            free(fname);
            break;
        }

        int num_replica_net;
        if (recv(thinf->socket, &num_replica_net, sizeof(int), MSG_WAITALL) != sizeof(int))
        {
            perror("Error en el método num_replica_net, no se ha detectado la replica correcta");
            free(fname);
            break;
        }

        int size_net;
        if (recv(thinf->socket, &size_net, sizeof(int), MSG_WAITALL) != sizeof(int))
        {
            perror("Error en el método recv, no se ha detectado la cantidad de caracteres necesarios");
            free(fname);
            break;
        }
        free(fname);
    }
    close(thinf->socket);
    free(thinf);
    return NULL;
}

int main(int argc, char *argv[])
{
    int s, s_conec, s_cliente;
    char codigo;
    unsigned int tam_dir;
    struct sockaddr_in dir_cliente;

    if (argc != 4)
    {
        perror("Error, el numero de argumentos es incorrectos");
        return -1;
    }
    else
    {
        mkdir(argv[1], 0755);
    }

    unsigned short puerto;
    if ((s = create_socket_srv(0, &puerto)) < 0)
    {
        perror("Errpr al inicializar el socket para aceptar conexiones");
        return -1;
    }

    s_cliente = create_socket_cln_by_name(argv[2], argv[3]);
    codigo = 'R';

    struct iovec iov[2];
    iov[0].iov_base = &codigo;
    iov[0].iov_len = sizeof(char);

    int puerto_net;
    puerto_net = htons(puerto);
    iov[1].iov_base = &puerto_net;
    iov[1].iov_len = sizeof(int);

    if (writev(s_cliente, iov, 2) < 0)
    {
        perror("Error al intentar escribir en writev con el socket correspondiente");
        close(s);
        close(s_cliente);
        return -1;
    }
    else
    {
        close(s_cliente);
    }

    pthread_t thid;
    pthread_attr_t atrib_th;
    pthread_attr_init(&atrib_th);
    pthread_attr_setdetachstate(&atrib_th, PTHREAD_CREATE_DETACHED);
    while (1)
    {
        tam_dir = sizeof(dir_cliente);
        if ((s_conec = accept(s, (struct sockaddr *)&dir_cliente, &tam_dir)) < 0)
        {
            perror("Error en el accept, el tamaño no es correcto");
            close(s);
            return -1;
        }
        else
        {
            thread_info *thinf = malloc(sizeof(thread_info));
            thinf->socket = s_conec;
            thinf->dir = argv[1];
            pthread_create(&thid, &atrib_th, servicio, thinf);
        }
    }
    close(s);
    return 0;
}