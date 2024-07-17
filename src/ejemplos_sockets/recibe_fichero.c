// EJEMPLO DE RECEPCIÓN DE UN FICHERO QUE USA mmap.
// PUEDE USARLO PARA LA OPERACIÓN DE ESCRITURA EN UN SERVIDOR.
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include "common_srv.h"

int main(int argc, char *argv[]) {
    int s, s_con, f;
    int tam_fich;
    void *p;

    if (argc!=3) {
        fprintf(stderr, "Uso: %s puerto fichero\n", argv[0]); return 1;
    }
    if ((s=create_socket_srv(atoi(argv[1]), NULL))<0) return 1;
    if ((s_con=accept(s, NULL, NULL))<0){
        perror("error en accept"); close(s); return 1;
    }
    if (recv(s_con, &tam_fich, sizeof(int), MSG_WAITALL)!=sizeof(int)) {
        perror("error en recv tam"); close(s); close(s_con); return 1;
    }
    tam_fich = ntohl(tam_fich);
    if ((f=open(argv[2], O_CREAT|O_TRUNC|O_RDWR, 0666))<0) {
        perror("error en creat"); close(s); close(s_con); return 1;
    }
    if (ftruncate(f,tam_fich)<0) {
        perror("error en ftruncate"); close(s); close(s_con); return 1;
    }
    if ((p = mmap(NULL, tam_fich, PROT_WRITE, MAP_SHARED, f, 0))==MAP_FAILED) {
        perror("error en mmap"); close(s); close(s_con); close(f); return 1;
    }
    close(f);
    if (recv(s_con, p, tam_fich, MSG_WAITALL)!=tam_fich) {
        perror("error en recv fichero"); close(s); close(s_con); return 1;
    }
    close(s_con);
    close(s);
    munmap(p, tam_fich);
    return 0;
}
