// EJEMPLO DE ENVÍO DE UN FICHERO CON sendfile.
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sendfile.h>

int main(int argc, char *argv[]) {
    int s, f;
    int tam_fich;
    struct stat st;

    if (argc!=4) {
        fprintf(stderr, "Uso: %s host puerto fichero\n", argv[0]); return -1;
    }
    if ((s=create_socket_cln_by_name(argv[1], argv[2]))<0) return -1;
    if ((f = open(argv[3], O_RDONLY))<0) {
        perror("error abriendo fichero"); return -1;
    }
    if (fstat(f, &st) == -1) {
        perror("error stat fichero"); close(f); return -1;
    }
    tam_fich = ntohl(st.st_size);
    // MSG_MORE: avisa de que se van a enviar más datos inmediatamente
    if (send(s, &tam_fich, sizeof(int), MSG_MORE)!=sizeof(int)) {
        perror("error send"); close(f); return -1;
    }
    if (sendfile(s, f, NULL, st.st_size)!=st.st_size) {
        perror("error sendfile"); close(f); return -1;
    }
    close(s);
    close(f);
}
