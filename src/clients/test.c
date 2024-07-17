// PROGRAMA PARA REALIZAR PRUEBAS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "mgfs.h"

static void prueba_conexion(unsigned int ip, unsigned short port) {
    struct sockaddr_in dir = {.sin_family=AF_INET, .sin_port=port, .sin_addr={ip}};
    int s;
    s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect(s, (struct sockaddr *)&dir, sizeof(dir)) < 0)
        perror("error ya que no se ha podido conectar con el servidor");
    else
        printf("OK se ha podido conectar con el servidor\n");
    close(s);
}


static char * leer_string(char *prompt) {
    int n;
    char *str;
    char *lin=NULL;
    size_t ll=0;
    fputs(prompt, stdout);
    n=getline(&lin, &ll, stdin);
    if (n<1) {ungetc(' ', stdin); return NULL;}
    n=sscanf(lin, "%ms", &str);
    free(lin);
    if (n!=1) return NULL;
    return str;
}
int leer_int(char *prompt) {
    int n, v;
    char *lin=NULL;
    size_t ll=0;
    fputs(prompt, stdout);
    n=getline(&lin, &ll, stdin);
    if (n<1) {ungetc(' ', stdin); return -1;}
    n=sscanf(lin, "%d", &v);
    free(lin);
    if (n!=1) return -1;
    return v;
}
int main(int argc, char *argv[]) {
    int n_fs=0, fs_id;
    int n_f=0, f_id;
    mgfs_fs *fs;
    mgfs_file *file;
    mgfs_fs **fs_list=NULL;
    mgfs_file **file_list=NULL;
    char *op;
    char *host, *port;
    int bs, rep, nfiles, n, ns, nb, nbls, tam, rd;
    char *fname, *buf;
    unsigned int ip;
    unsigned short puerto;
    unsigned int *ips;
    unsigned short *puertos;
    char car='A'-1;

    while (1) {
	op=leer_string("\nSeleccione operación (ctrl-D para terminar; en menús internos ctrl-D para volver a menú principal)\n\tX: coneXión con FS|D: Desconexión de FS\n\tC: Crear fichero|E: cErrar fichero|N: Nº ficheros|O: abrir fichero\n\tS: información Serv.\n\tA: Asigna serv. a bloque fich.|I: Info. serv. asignado a bloque\n\tW: escribe en fichero\n");
        if (op==NULL) break;
        switch(op[0]) {
            case 'X':
		host=leer_string("Introduzca el nombre del host del master: ");
                if (host==NULL) continue;
		port=leer_string("Introduzca el puerto del master: ");
                if (port==NULL) continue;
		bs=leer_int("Introduzca tamaño de bloque por defecto: ");
                if (bs==-1) continue;
		rep=leer_int("Introduzca factor de replicación por defecto: ");
                if (rep==-1) continue;
X                if ((fs=mgfs_connect(host, port, bs, rep))==NULL)
                    printf("error en mgfs_connect\n");
                else {
                    fs_list=realloc(fs_list, ++n_fs * sizeof(mgfs_fs *));
		    fs_list[n_fs-1]=fs;
                    printf("\nEstablecida conexión: ID del FS %d tambloque def. %d rep. factor def. %d\n", n_fs-1, mgfs_get_def_blocksize(fs), mgfs_get_def_rep_factor(fs));
		}
		free(host); free(port);
                break;
            case 'D':
		fs_id=leer_int("Introduzca el ID del FS: ");
                if (fs_id==-1) continue;
		if ((fs_id < 0) || (fs_id >= n_fs)) {
			printf("ID de FS inválido\n"); continue;
		}
                if ((mgfs_disconnect(fs_list[fs_id]))<0)
                    printf("error en mgfs_disconnect\n");
                else {
		    fs_list[fs_id]=NULL;
                    printf("\nDesconexión del FS con ID %d\n", fs_id);
		}
                break;
            case 'C':
		fs_id=leer_int("Introduzca el ID del FS: ");
                if (fs_id==-1) continue;
		if ((fs_id < 0) || (fs_id >= n_fs)) {
			printf("ID de FS inválido\n"); continue;
		}
		fname=leer_string("Introduzca el nombre del fichero: ");
                if (fname==NULL) continue;
		bs=leer_int("Introduzca tamaño de bloque del fichero: ");
                if (bs==-1) continue;
		rep=leer_int("Introduzca factor de replicación del fichero: ");
                if (rep==-1) continue;
                if ((file=mgfs_create(fs_list[fs_id], fname, bs, rep))==NULL)
                    printf("error en mgfs_create\n");
                else {
                    file_list=realloc(file_list, ++n_f * sizeof(mgfs_file *));
		    file_list[n_f-1]=file;
                    printf("creado fichero con ID %d: tambloque %d rep. factor %d\n", n_f-1, mgfs_get_blocksize(file), mgfs_get_rep_factor(file));
		}
		free(fname);
                break;
            case 'E':
		f_id=leer_int("Introduzca el ID del fichero: ");
                if (f_id==-1) continue;
		if ((f_id < 0) || (f_id >= n_f)) {
			printf("ID de fichero inválido\n"); continue;
		}
		file=file_list[f_id];
		file = (mgfs_file *)((long)file & ~0x1);
                if (mgfs_close(file)<0)
                    printf("error en mgfs_close\n");
                else {
		    file_list[f_id]=NULL;
                    printf("\nCierre del fichero con ID %d\n", f_id);
		}
                break;
            case 'N':
		fs_id=leer_int("Introduzca el ID del FS: ");
                if (fs_id==-1) continue;
		if ((fs_id < 0) || (fs_id >= n_fs)) {
			printf("ID de FS inválido\n"); continue;
		}
                if ((nfiles=_mgfs_nfiles(fs_list[fs_id]))<0)
                    printf("error en _mgfs_nfiles\n");
                else {
                    printf("\nFS con ID %d tiene %d ficheros\n", fs_id, nfiles);
		}
                break;
            case 'O':
		fs_id=leer_int("Introduzca el ID del FS: ");
                if (fs_id==-1) continue;
		if ((fs_id < 0) || (fs_id >= n_fs)) {
			printf("ID de FS inválido\n"); continue;
		}
		fname=leer_string("Introduzca el nombre del fichero: ");
                if (fname==NULL) continue;
                if ((file=mgfs_open(fs_list[fs_id], fname))==NULL)
                    printf("error en mgfs_open\n");
                else {
                    file_list=realloc(file_list, ++n_f * sizeof(mgfs_file *));
		    file_list[n_f-1]=(mgfs_file *)((long)file | 0x1);
                    printf("abierto fichero con ID %d: tambloque %d rep. factor %d\n", n_f-1, mgfs_get_blocksize(file), mgfs_get_rep_factor(file));
		}
		free(fname);
                break;
            case 'S':
                fs_id=leer_int("Introduzca el ID del FS: ");
                if (fs_id==-1) continue;
                if ((fs_id < 0) || (fs_id >= n_fs)) {
                        printf("ID de FS inválido\n"); continue;
                }
                ns=leer_int("Introduzca el nº de servidor: ");
                if (ns==-1) continue;
                if (_mgfs_serv_info(fs_list[fs_id], ns, &ip, &puerto)<0)
                    printf("error en _mgfs_serv_info\n");
                else {
                    printf("\nInfo de servidor %d de FS con ID %d: ip %u puerto %u\n", ns, fs_id, ip, puerto);
		    prueba_conexion(ip, puerto);
		}
                break;
            case 'A':
		f_id=leer_int("Introduzca el ID del fichero: ");
                if (f_id==-1) continue;
		if ((f_id < 0) || (f_id >= n_f)) {
			printf("ID de fichero inválido\n"); continue;
		}
		file=file_list[f_id];
		file = (mgfs_file *)((long)file & ~0x1);
                rep= mgfs_get_rep_factor(file);
                ips=malloc(rep*sizeof(unsigned int));
                puertos=malloc(rep*sizeof(unsigned short int));
                if (_mgfs_alloc_next_block(file, ips, puertos)<0)
                    printf("error en _mgfs_alloc_next_block\n");
                else {
                    for (int i=0; i<rep; i++) {
                        printf("\nInfo de servidor asignado a réplica %d: ip %u puerto %u\n", i, ips[i], puertos[i]);
		        prueba_conexion(ips[i], puertos[i]);
		    }
		}
		free(ips);
		free(puertos);
                break;
            case 'I':
		f_id=leer_int("Introduzca el ID del fichero: ");
                if (f_id==-1) continue;
		if ((f_id < 0) || (f_id >= n_f)) {
			printf("ID de fichero inválido\n"); continue;
		}
		nb=leer_int("Introduzca el nº de bloque: ");
		file=file_list[f_id];
		file = (mgfs_file *)((long)file & ~0x1);
                rep= mgfs_get_rep_factor(file);
                ips=malloc(rep*sizeof(unsigned int));
                puertos=malloc(rep*sizeof(unsigned short int));
                if (_mgfs_get_block_allocation(file, nb, ips, puertos)<0)
                    printf("error en _mgfs_get_block_allocation\n");
                else {
                    for (int i=0; i<rep; i++) {
                        printf("\nInfo de servidor asignado a réplica %d de bloque %d: ip %u puerto %u\n", i, nb, ips[i], puertos[i]);
		        prueba_conexion(ips[i], puertos[i]);
		    }
		}
		free(ips);
		free(puertos);
                break;
            case 'W':
		f_id=leer_int("Introduzca el ID del fichero: ");
                if (f_id==-1) continue;
		if ((f_id < 0) || (f_id >= n_f)) {
			printf("ID de fichero inválido\n"); continue;
		}
		file=file_list[f_id];
		rd = ((long) file & 0x1) ? 1 : 0;
		if (rd) {
			printf("no se puede escribir fich. abierto para lectura\n"); continue;
		}
		file = (mgfs_file *)((long)file & ~0x1);
		nbls=leer_int("Introduzca el tamaño de la escritura en nº bloques: ");
                bs= mgfs_get_blocksize(file);
		tam=nbls*bs;
                buf=malloc(tam);
		for (int i=0, b=0; i<tam; i++) {
		    if (i%bs==0) 
		        printf("en el bloque %d se escribe %c\n", b++, ++car);
                    buf[i]=car;
		}
                if ((n=mgfs_write(file, buf, tam))<0)
                    printf("error en mgfs_write\n");
                else {
                    if (n!=tam)
                        printf("mgfs_write devuelve %d en vez de %d\n", n, tam);
		    else
                        printf("mgfs_write devuelve %d\n", n);
		}
		free(buf);
                break;
            default:
                    printf("operación no válida\n");
            }
        if (op) free(op);
    };
    return 0;
}

