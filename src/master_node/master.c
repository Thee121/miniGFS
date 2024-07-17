#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/uio.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "master.h"
#include "common.h"
#include "common_srv.h"
#include "map.h"
#include "array.h"

typedef struct server_info
{
    unsigned long ip;
    unsigned short puerto;
} server_info;

array *servers;

typedef struct file_info
{
    int tam_bloque;
    int rep_factor;
    char *fname;
    array *block_list;
    map *server_map;
} file_info;

map *mapa_archivos;

// información que se la pasa el thread creado
typedef struct thread_info
{
    int socket;
    unsigned long ip;
} thread_info;

void *servicio(void *arg)
{
    thread_info *thinf = arg; // Informacion del thread a tratar. Se importa la información de argumento

    char codigo;
    int longitud;
    char *fname;

    // si recv returnea numero<=0 , el cliente ha forzado la interrupcion de la conexión;
    // Implementado con MSG_WAITALL para que no tenga problemas el servicio de llegada de información
    while (1)
    {
        // Todas las peticiones comienzan con el código de operación
        if (recv(thinf->socket, &codigo, sizeof(char), MSG_WAITALL) != sizeof(char))
        {
            break;
        }
        else
        {
            switch (codigo)
            {
            case 'C':
            { // Crea el fichero pedido
                file_info  *f;
                f = malloc(sizeof(file_info ));

                if (recv(thinf->socket, &longitud, sizeof(int), MSG_WAITALL) != sizeof(int))
                {
                    perror("Error en secv, la longitud es incorrecta [1]");
                    free(f);
                    break;
                }
                else
                {
                    longitud = ntohl(longitud);
                    f->fname = malloc(longitud + 1);
                }

                if (recv(thinf->socket, f->fname, longitud, MSG_WAITALL) != longitud)
                {
                    perror("Error en secv, la longitud es incorrecta [2]");
                    free(f->fname);
                    free(f);
                    break;
                }
                else
                {
                    f->fname[longitud] = '\0';
                }

                if (recv(thinf->socket, &f->tam_bloque, sizeof(int), MSG_WAITALL) != sizeof(int))
                {
                    perror("Error en secv, el tamaño de bloque es incorrecto");
                    free(f->fname);
                    free(f);
                    break;
                }
                else if (recv(thinf->socket, &f->rep_factor, sizeof(int), MSG_WAITALL) != sizeof(int))
                {
                    perror("Error en secv, el factor es incorrecto");
                    free(f->fname);
                    free(f);
                    break;
                }
                else
                {
                    f->block_list = array_create(1);
                    f->server_map = map_create(key_string, 1);
                }
                int resultado = map_put(mapa_archivos, f->fname, f);

                if (write(thinf->socket, &resultado, sizeof(int)) < 0)
                {
                    perror("Error en write, no se ha podido escribir por el socket");
                    int resultado_net = htonl(resultado);
                    write(thinf->socket, &resultado_net, sizeof(int));
                    free(f->fname);
                    free(f);
                    break;
                }
                break;
            }

            case 'N':
            { // Calcula el numero de ficheros en el sistema
                int num_ficheros = map_size(mapa_archivos);
                if (write(thinf->socket, &num_ficheros, sizeof(int)) < 0)
                {
                    perror("Error en write al intentar escribir el numero de ficheros");
                    break;
                }
                break;
            }

            case 'O':
            { // Abre el fichero pedido del sistema

                if (recv(thinf->socket, &longitud, sizeof(int), MSG_WAITALL) != sizeof(int))
                {
                    perror("Error, la longitud de la informacion al abrir un fichero es incorrecto");
                    break;
                }
                longitud = ntohl(longitud);

                fname = malloc(longitud + 1);

                if (recv(thinf->socket, fname, longitud, MSG_WAITALL) != longitud)
                {
                    perror("Error en recv, la longitud es incorrecta al intentar crear la memoria para el fichero");
                    free(fname);
                    break;
                }

                fname[longitud] = '\0';

                int resultado = 0;
                file_info *fAux;
                fAux = map_get(mapa_archivos, fname, &resultado);
                free(fname);

                if (resultado == -1)
                {
                    perror("Error al realizar la operacion map_get, no existe el archivo");
                    int resultadoN = htonl(resultado);
                    write(thinf->socket, &resultadoN, sizeof(int));
                    break;
                }

                struct iovec iov[3];
                iov[0].iov_base = &resultado;
                iov[0].iov_len = sizeof(int);

                iov[1].iov_base = &fAux->tam_bloque;
                iov[1].iov_len = sizeof(int);

                iov[2].iov_base = &fAux->rep_factor;
                iov[2].iov_len = sizeof(int);

                if (writev(thinf->socket, iov, 3) < 0)
                {
                    perror("Error en writev, no se ha mandado la informacion correctamente");
                    break;
                }
                else
                {
                    break;
                }
            }

            case 'I':
            { // Obtiene la información de localización
                int serverN, serverNum;
                if (recv(thinf->socket, &serverN, sizeof(int), MSG_WAITALL) != sizeof(int)){
                    perror("Error, el tamaño del serverN no es correcto");
                    break;
                }
                serverNum = ntohl(serverN);

                int resultado;
                server_info *server;
                server = array_get(servers, serverNum, &resultado);
                if (resultado == -1)
                {
                    perror("error en array_get, el servidor no existe");
                    int resultado_net = htonl(resultado);
                    write(thinf->socket, &resultado_net, sizeof(int));
                    break;
                }

                int puerto_net = htonl(server->puerto);
                int ip_net = htonl(server->ip);

                struct iovec iov[2];
                iov[0].iov_base = &puerto_net;
                iov[0].iov_len = sizeof(int);

                iov[1].iov_base = &ip_net;
                iov[1].iov_len = sizeof(int);

                if (writev(thinf->socket, iov, 2) < 0)
                {
                    perror("Error en writev al intentar escribir por el socket para devolver informacion de server");
                    break;
                }
                break;
            }

            case 'R':
            { // Da de alta un servidor en el sistema
                unsigned short puerto;
                int puertoN;
                if (recv(thinf->socket, &puertoN, sizeof(int), MSG_WAITALL) != sizeof(int))
                {
                    perror("Error ya que la longitud del puerto no es correcto");
                    break;
                }
                else
                {
                    puerto = ntohs(puertoN);
                    server_info *servidorNuevo;
                    servidorNuevo = malloc(sizeof(server_info));
                    servidorNuevo->ip = thinf->ip;
                    servidorNuevo->puerto = puerto;

                    array_append(servers, servidorNuevo);
                    return NULL;
                }
            }

            default:
                perror("Error, no se ha reconocido el codigo del comando a realizar");
                break;
            }
        }
    }
    close(thinf->socket);
    free(thinf);
    return NULL;
}

int main(int argc, char *argv[])
{
    int s, s_conec;
    unsigned int tam_dir;
    struct sockaddr_in dir_cliente;

    if (argc != 2)
    {
        perror("Error, numero de argumentos incorrecto");
        return -1;
    }
    else if ((s = create_socket_srv(atoi(argv[1]), NULL)) < 0)
    {
        perror("Error al intentar crear el socket con los parametros dados");
        return -1;
    }
    else
    {
        servers = array_create(1);
    }

    mapa_archivos = map_create(key_string, 1);

    // Prepara los atributos adecuados para crear thread "detached"
    pthread_t thid;
    pthread_attr_t atrib_th;
    pthread_attr_init(&atrib_th);
    pthread_attr_setdetachstate(&atrib_th, PTHREAD_CREATE_DETACHED);
    while (1)
    {
        tam_dir = sizeof(dir_cliente);
        if ((s_conec = accept(s, (struct sockaddr *)&dir_cliente, &tam_dir)) < 0)
        {
            perror("Error en la operacion accept");
            close(s);
            return -1;
        }

        // crea el thread de servicio
        thread_info *thinf = malloc(sizeof(thread_info));
        thinf->socket = s_conec;
        thinf->ip = dir_cliente.sin_addr.s_addr;
        pthread_create(&thid, &atrib_th, servicio, thinf);
    }
    close(s);
    return 0;
}