// EJEMPLO DE CLIENTE QUE ENVÍA CON UNA SOLA OPERACIÓN UN ENTERO,
// UN STRING Y UN ARRAY BINARIO. PARA ENVIAR ESTOS DOS ÚLTIMOS, AL
// SER DE TAMAÑO VARIABLE, TRANSMITE ANTES SU TAMAÑO.
// COMO RESPUESTA RECIBE UN ENTERO Y UN ARRAY DE TAMAÑO VARIABLE.
// PARA SIMULAR VARIAS PETICIONES, REPITE VARIAS VECES ESA ACCIÓN.
// PUEDE USARLO COMO BASE PARA LA BIBLIOTECA DE CLIENTE.
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include "common_cln.h"

#define NUM_REQ 3 // REPITE VARIAS VECES (COMO SI FUERAN MÚLTIPLES PETICIONES)
		  
int main(int argc, char *argv[]) {
    int s;
    if (argc!=3) {
        fprintf(stderr, "Uso: %s servidor puerto\n", argv[0]);
        return -1;
    }
    // inicializa el socket y se conecta al servidor
    if ((s=create_socket_cln_by_name(argv[1], argv[2]))<0) return -1;

    // datos a enviar
    int entero = 12345;
    char *string = "abcdefghijklmnopqrstuvwxyz";
    // podría ser una imagen, un hash, texto cifrado...
    unsigned char array[] = {0x61, 0x62, 0x0, 0x9, 0xa, 0x41, 0x42};
    struct iovec iov[5]; // hay que enviar 5 elementos
    int nelem;

    // los envía varias veces como si fueran sucesivas peticiones del cliente
    // que mantiene una conexión persistente
    for (int i=0; i<NUM_REQ; i++) {
        nelem = 0;
        // preparo el envío del entero convertido a formato de red
        int entero_net = htonl(entero);
        iov[nelem].iov_base=&entero_net;
        iov[nelem++].iov_len=sizeof(int);

        // preparo el envío del string mandando antes su longitud
        int longitud_str = strlen(string); // no incluye el carácter nulo
        int longitud_str_net = htonl(longitud_str);
        iov[nelem].iov_base=&longitud_str_net;
        iov[nelem++].iov_len=sizeof(int);
        iov[nelem].iov_base=string; // no se usa & porque ya es un puntero
        iov[nelem++].iov_len=longitud_str;

        // preparo el envío del array mandando antes su longitud
        int longitud_arr_net = htonl(sizeof(array));
        iov[nelem].iov_base=&longitud_arr_net;
        iov[nelem++].iov_len=sizeof(int);
        iov[nelem].iov_base=array; // no se usa & porque ya es un puntero
        iov[nelem++].iov_len=sizeof(array);

        // modo de operación de los sockets asegura que si no hay error
        // se enviará todo (misma semántica que los "pipes")
        if (writev(s, iov, nelem)<0) {
            perror("error en writev"); close(s); return -1;
        }
	// recibe el entero y un array como respuesta
	if (recv(s, &entero, sizeof(int), MSG_WAITALL) != sizeof(int)) {
            perror("error en recv int"); close(s); return -1;
        }
	entero = ntohl(entero);
        printf("Recibido entero: %d\n", entero);
	int longitud;
        if (recv(s, &longitud, sizeof(int), MSG_WAITALL)!=sizeof(int)){
            perror("error en recv longitud"); close(s); return -1;
        }
        longitud = ntohl(longitud);
        unsigned char *array = malloc(longitud); // no usa un terminador
        // llega el array
        if (recv(s, array, longitud, MSG_WAITALL)!=longitud) {
            perror("error en recv array"); close(s); return -1;
        }
        printf("Recibido array: ");
        for (int i=0; i<longitud; i++) printf("%02x", array[i]);
        printf("\n");

	// cambio algunos valores para la próxima "petición"
	++entero;
	++array[0];
    }
    close(s); // cierra el socket
    return 0;
}
