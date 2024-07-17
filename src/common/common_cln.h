// PUEDE INCLUIR AQUÍ LA DEFINICIÓN DE FUNCIONALIDADES COMUNES
// RELACIONADAS CON UN CLIENTE DE SOCKETS (LA BIBLIOTECA Y LOS SERVIDORES
// ACTÚAN DE CLIENTES).
//
// INICIALMENTE, INCLUYE LA DEFINICIÓN DE FUNCIONES QUE CREAN Y CONECTAN
// UN SOCKET, QUE PUEDE USAR SI LO CONSIDERA OPORTUNO.
//
// PUEDE MODIFICAR EL FICHERO A DISCRECIÓN.

#ifndef _COMMON_CLN_H
#define _COMMON_CLN_H        1

// crea socket y se conecta al servidor traduciendo previamente su nombre
int create_socket_cln_by_name(const char *host, const char *port);

// crea socket y se conecta al servidor a partir de IP y puerto en formato red
int create_socket_cln_by_addr(unsigned int ip, unsigned short port);

#endif // _COMMON_CLN_H

