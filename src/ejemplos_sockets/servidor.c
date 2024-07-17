// EJEMPLO DE SERVIDOR MULTITHREAD QUE RECIBE PETICIONES DE LOS CLIENTES.
// PUEDE USARLO COMO BASE PARA DESARROLLAR EL MASTER Y EL SERVER DE LA PRÁCTICA.
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <pthread.h>
#include "common_srv.h"


// información que se la pasa el thread creado
typedef struct thread_info {
    int socket; // añadir los campos necesarios
} thread_info;

// función del thread
void *servicio(void *arg){
    int entero;
    int longitud;
    char *string;
    unsigned char *array;
    thread_info *thinf = arg; // argumento recibido

    // si recv devuelve <=0 el cliente ha cortado la conexión;
    // recv puede devolver menos datos de los solicitados
    // (misma semántica que el "pipe"), pero con MSG_WAITALL espera hasta que
    // se hayan recibido todos los datos solicitados o haya habido un error.
    while (1) {
        // cada "petición" comienza con un entero
        if (recv(thinf->socket, &entero, sizeof(int), MSG_WAITALL)!=sizeof(int))
            break;
        entero = ntohl(entero);
        printf("Recibido entero: %d\n", entero);

        // luego llega el string, que viene precedido por su longitud
        if (recv(thinf->socket, &longitud, sizeof(int), MSG_WAITALL)!=sizeof(int))
            break;
        longitud = ntohl(longitud);
        string = malloc(longitud+1); // +1 para el carácter nulo
        // ahora sí llega el string
        if (recv(thinf->socket, string, longitud, MSG_WAITALL)!=longitud)
            break;
        string[longitud]='\0';       // añadimos el carácter nulo
        printf("Recibido string: %s\n", string);

        // y finalmente llega el array binario precedido de su longitud
        if (recv(thinf->socket, &longitud, sizeof(int), MSG_WAITALL)!=sizeof(int))
            break;
        longitud = ntohl(longitud);
        array = malloc(longitud); // no usa un terminador
        // llega el array
        if (recv(thinf->socket, array, longitud, MSG_WAITALL)!=longitud)
            break;
        printf("Recibido array: ");
        for (int i=0; i<longitud; i++) printf("%02x", array[i]);
        printf("\n");

        // envía un entero y un array como respuesta
        struct iovec iov[3]; // hay que enviar 3 elementos
        int nelem = 0;

        ++entero; // cambio el int recibido
        // preparo el envío del entero convertido a formato de red
        int entero_net = htonl(entero);
        iov[nelem].iov_base=&entero_net;
        iov[nelem++].iov_len=sizeof(int);

        // cambio el array recibido
        for (int i=0; i<longitud; ++array[i], i++); 

        int longitud_arr_net = htonl(longitud);
        iov[nelem].iov_base=&longitud_arr_net;
        iov[nelem++].iov_len=sizeof(int);
        iov[nelem].iov_base=array; // no se usa & porque ya es un puntero
        iov[nelem++].iov_len=longitud;

        if (writev(thinf->socket, iov, 3)<0) {
            perror("error en writev");
            close(thinf->socket);
            return NULL;
        }

    }
    close(thinf->socket);
    free(thinf);
    printf("conexión del cliente cerrada\n");
    return NULL;
}
int main(int argc, char *argv[]) {
    int s, s_conec;
    unsigned int tam_dir;
    struct sockaddr_in dir_cliente;

    if (argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return -1;
    }
    // inicializa el socket y lo prepara para aceptar conexiones
    if ((s=create_socket_srv(atoi(argv[1]), NULL)) < 0) return -1;

    // prepara atributos adecuados para crear thread "detached"
    pthread_t thid;
    pthread_attr_t atrib_th;
    pthread_attr_init(&atrib_th); // evita pthread_join
    pthread_attr_setdetachstate(&atrib_th, PTHREAD_CREATE_DETACHED);
    while(1) {
        tam_dir=sizeof(dir_cliente);
        // acepta la conexión
        if ((s_conec=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
            perror("error en accept");
            close(s);
            return -1;
        }
        printf("conectado cliente con ip %u y puerto %u (formato red)\n",
                dir_cliente.sin_addr.s_addr, dir_cliente.sin_port);
        // crea el thread de servicio
        thread_info *thinf = malloc(sizeof(thread_info));
        thinf->socket=s_conec;
        pthread_create(&thid, &atrib_th, servicio, thinf);
    }
    close(s); // cierra el socket general
    return 0;
}
