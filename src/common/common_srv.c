// PUEDE INCLUIR AQUÍ LA IMPLEMENTACIÓN DE FUNCIONALIDADES COMUNES
// RELACIONADAS CON UN SERVIDOR DE SOCKETS (EL MAESTRO Y LOS SERVIDORES
// ACTÚAN DE SERVIDORES).
//
// INICIALMENTE, INCLUYE LA IMPLEMENTACIÓN DE UNA FUNCIÓN QUE CREA UN SOCKET DE
// SERVIDOR, QUE PUEDE USAR SI LO CONSIDERA OPORTUNO.
//
// PUEDE MODIFICAR EL FICHERO A DISCRECIÓN.

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include "common_srv.h"

// Inicializa el socket y lo prepara para aceptar conexiones.
// Parámetro 1º corresponde al puerto solicitado en formato nativo;
// Si 2º parámetro!=NULL, devuelve en él el puerto asignado, en formato de red,
// que será el mismo que el pedido, excepto si este era 0.
int create_socket_srv(unsigned short pto_pedido, unsigned short *pto_asignado) {
    int s;
    struct sockaddr_in dir;
    unsigned int td=sizeof(dir);
    int opcion=1;
    if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("error creando socket"); return -1;
    }
    // Para reutilizar puerto inmediatamente si se rearranca el servidor
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion))<0){
        perror("error en setsockopt"); close(s); return -1;
    }
    // asocia el socket al puerto especificado (si 0, SO elige el puerto)
    dir.sin_addr.s_addr=INADDR_ANY;
    dir.sin_port=htons(pto_pedido);
    dir.sin_family=PF_INET;
    if (bind(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        perror("error en bind"); close(s); return -1;
    }
    // devuelve el puerto asignado, en formato de red,
    // si lo han pedido (2º parámetro!=NULL);
    if (pto_asignado) {
        if (getsockname(s, (struct sockaddr *) &dir, &td)<0) {
            perror("error en getsockname"); close(s); return -1;
        }
        *pto_asignado = dir.sin_port;
    }
    // establece el nº máx. de conexiones pendientes de aceptar
    if (listen(s, 5) < 0) {
        perror("error en listen"); close(s); return -1;
    }
    return s;
}
