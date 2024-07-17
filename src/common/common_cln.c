// PUEDE INCLUIR AQUÍ LA IMPLEMENTACIÓN DE FUNCIONALIDADES COMUNES
// RELACIONADAS CON UN CLIENTE DE SOCKETS (LA BIBLIOTECA Y LOS SERVIDORES
// ACTÚAN DE CLIENTES).
//
// INICIALMENTE, INCLUYE LA IMPLEMENTACIÓN DE FUNCIONES QUE CREAN Y CONECTAN
// UN SOCKET, QUE PUEDE USAR SI LO CONSIDERA OPORTUNO.
//
// PUEDE MODIFICAR EL FICHERO A DISCRECIÓN.

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// función común que crea socket y se conecta al servidor
static int create_socket_cln(const struct sockaddr *dir, int dirlen) {
    int s;
    if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("error creando socket"); return -1;
    }
    // realiza la conexión
    if (connect(s, dir, dirlen) < 0) {
        perror("error en connect"); close(s); return -1;
    }
    return s;
}
// crea socket y se conecta al servidor traduciendo previamente su nombre
int create_socket_cln_by_name(const char *host, const char *port) {
    // para indicar que solo trabaja con IPv4
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;    /* solo IPv4 */
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;
    // obtiene la dirección TCP remota
    if (getaddrinfo(host, port, &hints, &res)!=0) {
        perror("error en getaddrinfo"); return -1;
    }
    int s=create_socket_cln(res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    return s;
}
// crea socket y se conecta al servidor a partir de IP y puerto en formato red
int create_socket_cln_by_addr(unsigned int ip, unsigned short port) {
    struct sockaddr_in dir = {.sin_family=AF_INET, .sin_port=port, .sin_addr={ip}};
    return create_socket_cln((struct sockaddr *)&dir, sizeof(dir));
}
