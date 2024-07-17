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
        int num_replica = ntohl(num_replica_net);

        int size_net;
        if (recv(thinf->socket, &size_net, sizeof(int), MSG_WAITALL) != sizeof(int))
        {
            perror("Error en el método recv, no se ha detectado la cantidad de caracteres necesarios");
            free(fname);
            break;
        }
        int size = ntohl(size_net);

        char dirname[256];
        mkdir(dirname, 0755);

        char filename[256];

        int archivo_id = open(filename, O_RDWR | O_CREAT, 0666);

        if (ftruncate(archivo_id, size) < 0)
        {
            perror("Error en el método ftruncate, ha habido un error con el archivo");
            break;
        }
        void *p;
        if ((p = mmap(NULL, size, PROT_WRITE, MAP_SHARED, archivo_id, 0)) == MAP_FAILED)
        {
            perror("Error al intentar crear el mapa mmap");
            close(archivo_id);
            break;
        }
        else
        {
            close(archivo_id);
        }

        if (recv(thinf->socket, p, size, MSG_WAITALL) != size){
            perror("Error en recv, el socket no es del tamaño correcto");
            break;
        }
        else if (num_replica == 0)
        {
            int num_replicas_net;
            if (recv(thinf->socket, &num_replicas_net, sizeof(int), MSG_WAITALL) != sizeof(int))
            {
                perror("Error en recv, el numero de replicas no es del tamaño correcto");
                break;
            }
            int num_replicas = ntohl(num_replicas_net);

            unsigned int *ips = malloc(num_replicas * sizeof(unsigned int));
            unsigned short *ports = malloc(num_replicas * sizeof(unsigned short));
            if (recv(thinf->socket, ips, num_replicas * sizeof(unsigned int), MSG_WAITALL) != num_replicas * sizeof(unsigned int))
            {
                perror("Error en recv, el numero de replicas local no es del tamaño correcto");
                break;
            }
            else if (recv(thinf->socket, ports, num_replicas * sizeof(unsigned short), MSG_WAITALL) != num_replicas * sizeof(unsigned short))
            {
                perror("Error en recv, el tamaño no es correcto");
                break;
            }
            int bytes_escritos_total = 0;
            int bytes_escritos = 0;

            int contador1 = 0;
            while (contador1 < num_replicas)
            {

                int sockfd = create_socket_cln_by_addr(ips[contador1], ports[contador1]);
                if (sockfd < 0)
                {
                    perror("Error en el metodo create_socket_cln_by_addr");
                    return NULL;
                }

                struct iovec iov[6];

                iov[0].iov_base = &longitud_fname_net;
                iov[0].iov_len = sizeof(int);

                iov[1].iov_base = fname;
                iov[1].iov_len = longitud_fname;

                iov[2].iov_base = &n_bloque_net;
                iov[2].iov_len = sizeof(int);

                int n_replica_net = htonl(contador1);
                iov[3].iov_base = &n_replica_net;
                iov[3].iov_len = sizeof(int);

                iov[4].iov_base = &size_net;
                iov[4].iov_len = sizeof(int);

                iov[5].iov_base = p;
                iov[5].iov_len = size;

                if (writev(sockfd, iov, 6) < 0)
                {
                    perror("Error en writev al intentar mandar la estructura IOV");
                    close(sockfd);
                    return NULL;
                }

                if (read(sockfd, &bytes_escritos, sizeof(int)) != sizeof(int))
                {
                    perror("Error al intentar leer los bytes escritos del socket [1]");
                    close(sockfd);
                    return NULL;
                }

                bytes_escritos = ntohl(bytes_escritos);
                if (bytes_escritos != size)
                {
                    perror("Error al intentar leer los bytes escritos del socket [2]");
                    close(sockfd);
                    return NULL;
                }
                bytes_escritos_total += bytes_escritos;
                contador1 = contador1 + 1;
            }

            int bytes_escritos_net = htonl(size);
            if (write(thinf->socket, &bytes_escritos_net, sizeof(int)) < 0)
            {
                perror("Error al intentar escribir byte en el socket [1]");
                break;
            }
            free(ips);
            free(ports);
        }
        else
        {
            int bytes_escritos = htonl(size);
            if (write(thinf->socket, &bytes_escritos, sizeof(int)) < 0)
            {
                perror("Error al intentar escribir bytes en el socket [2] ");
                break;
            }
        }
        free(fname);
    }
    close(thinf->socket);
    free(thinf);
    return NULL;
}

int main(int argc, char *argv[])
{
    int s, s_conec, s_cli;
    char codigo;
    unsigned int tam_dir;
    struct sockaddr_in dir_cliente;

    if (argc != 4)
    {
        perror("Numero de argumentos incorrectos");
        return -1;
    }
    else
    {
        mkdir(argv[1], 0755);
    }

    // inicializa el socket y lo prepara para aceptar conexiones
    unsigned short puerto;
    if ((s = create_socket_srv(0, &puerto)) < 0)
        return -1;

    // crea el socket "cliente", envia  código de operación y el puerto y cierra el socket correspondiente.
    s_cli = create_socket_cln_by_name(argv[2], argv[3]);
    codigo = 'L';

    struct iovec iov[2];
    iov[0].iov_base = &codigo;
    iov[0].iov_len = sizeof(char);

    int puerto_net;
    puerto_net = htons(puerto);
    iov[1].iov_base = &puerto_net;
    iov[1].iov_len = sizeof(int);

    if (writev(s_cli, iov, 2) < 0)
    {
        perror("Error al intentar escribir en writev con el socket correspondiente");
        close(s);
        close(s_cli);
        return -1;
    }
    else
    {
        close(s_cli);
    }

    // prepara atributos crear thread "detached"
    pthread_t thid;
    pthread_attr_t atrib_th;
    pthread_attr_init(&atrib_th);
    pthread_attr_setdetachstate(&atrib_th, PTHREAD_CREATE_DETACHED);
    while (1)
    {
        tam_dir = sizeof(dir_cliente);
        // acepta el inicio de la conexión
        if ((s_conec = accept(s, (struct sockaddr *)&dir_cliente, &tam_dir)) < 0)
        {
            perror("Error en el accept, el tamaño no es correcto");
            close(s);
            return -1;
        }
        else
        {
            // crea el thread "servicio" en el socket correspondiente
            thread_info *thinf = malloc(sizeof(thread_info));
            thinf->socket = s_conec;
            thinf->dir = argv[1];
            pthread_create(&thid, &atrib_th, servicio, thinf);
        }
    }
    close(s);
    return 0;
}