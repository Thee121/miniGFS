// EJEMPLO DE SERVIDOR BÁSICO QUE RECIBE UN ENTERO Y LO ENVÍA COMO
// RESPUESTA AL CLIENTE DESPUÉS DE INCREMENTARLO.

//#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "common_srv.h"

int main(int argc, char *argv[]) {
    int s, s_conec, entero;
    struct sockaddr_in dir_cliente;
    unsigned int tam_dir = sizeof(dir_cliente);

    if (argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return -1;
    }
    // inicializa el socket y lo prepara para aceptar conexiones
    if ((s=create_socket_srv(atoi(argv[1]), NULL)) < 0) return -1;
    while (1) {
        // acepta la conexión
        if ((s_conec=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
            perror("error en accept"); close(s); return -1;
        }
        if (recv(s_conec, &entero, sizeof(int), MSG_WAITALL)!=sizeof(int)){
            perror("error en recv"); close(s_conec); close(s); return -1;
        }
        entero = ntohl(entero); // a formato de host
        printf("Recibido entero: %d\n", entero);
	++entero;
        entero = ntohl(entero); // a formato de red
        if (write(s_conec, &entero, sizeof(int))<0) {
            perror("error en write"); close(s_conec); close(s); return -1;
        }
        close(s_conec);
    }
    close(s); // cierra el socket general
    return 0;
}
