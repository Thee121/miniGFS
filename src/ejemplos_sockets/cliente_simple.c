// EJEMPLO DE CLIENTE BÁSICO QUE ENVÍA UN ENTERO AL SERVIDOR Y
// RECIBE UN ENTERO DEL MISMO.
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common_cln.h"

int main(int argc, char *argv[]) {
    int s;
    if (argc!=3) {
        fprintf(stderr, "Uso: %s servidor puerto\n", argv[0]);
        return -1;
    }
    // inicializa el socket y se conecta al servidor
    if ((s=create_socket_cln_by_name(argv[1], argv[2]))<0) return -1;

    int entero = htonl(12345); // a formato de red
    if (write(s, &entero, sizeof(int))<0) {
        perror("error en write"); close(s); return -1;
    }
    if (recv(s, &entero, sizeof(int), MSG_WAITALL) != sizeof(int)) {
        perror("error en recv"); close(s); return -1;
    }
    printf("recibido %d\n", ntohl(entero)); // a formato de host
    close(s); // cierra el socket
    return 0;
}
