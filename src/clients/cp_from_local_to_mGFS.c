// COPIA UN FICHERO LOCAL AL miniGFS USANDO EL TAMAÑO DE BLOQUE Y EL
// FACTOR DE REPLICACIÓN ESPECIFICADOS.
// RECUERDE QUE EL FICHERO PARALELO TENDRÁ ASIGNADO UN NÚMERO ENTERO DE
// BLOQUES POR LO QUE, SI EL TAMAÑO DEL FICHERO NO ES MÚLTIPLO DEL BLOQUE,
// LA PARTE FINAL DEL FICHERO NO SERÁ SIGNIFICATIVA.
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "mgfs.h"

int main(int argc, char *argv[]) {
    if (argc!=7) {
        fprintf(stderr, "Uso: %s host puerto fichero_local fichero_mgfs tam_bl rep_factor\n", argv[0]);
        return -1;
    }
    mgfs_fs *mfs; // descriptor de sistema de ficheros
    // Se conecta al sistema de ficheros especificado con los valores por
    // defecto en el tamaño de bloque y en el factor de replicación indicados
    if ((mfs = mgfs_connect(argv[1], argv[2], atoi(argv[5]), atoi(argv[6]))) == NULL) {
        fprintf(stderr, "No puede conectarse a miniGFS\n"); return -1;
    }
    int fd;
    if ((fd = open(argv[3], O_RDONLY)) < 0) {
        perror("No puede abrir el fichero local\n"); return -1;
    }
    mgfs_file *mf; // descriptor de fichero
    // Se crea un fichero con los valores por defecto en el tamaño de bloque
    // y factor de replicación
    if ((mf=mgfs_create(mfs, argv[4], 0, 0))==NULL) {
        printf("error en mgfs_create\n");
    }
    int n, bs = mgfs_get_blocksize(mf);
    char buf[bs];
    while ((n=read(fd, buf, bs)) > 0) {
	// si última escritura menor que tam. bloque, se rellena a 0 el resto
        if (n<bs) memset(&buf[n], 0, bs-n);
        mgfs_write(mf, buf, bs); // se escriben siempre bloques completos
    }
    close(fd);
    mgfs_close(mf);
    mgfs_disconnect(mfs);
    return 0;
}
