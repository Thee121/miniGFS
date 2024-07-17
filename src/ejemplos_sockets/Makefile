CFLAGS=-Wall

PROGRS=cliente_simple servidor_simple cliente servidor envia_fichero recibe_fichero

all:  $(PROGRS)

cliente_simple.o cliente.o envia_fichero.o common_cln.o: common_cln.h
servidor_simple.o servidor.o recibe_fichero.o common_srv.o: common_srv.h

cliente_simple: cliente_simple.o common_cln.o
	$(CC) -o $@ $< common_cln.o

cliente: cliente.o common_cln.o
	$(CC) -o $@ $< common_cln.o

envia_fichero: cliente.o common_cln.o
	$(CC) -o $@ $< common_cln.o

servidor_simple: servidor_simple.o common_srv.o
	$(CC) -o $@ $< common_srv.o

servidor: servidor.o common_srv.o
	$(CC) -o $@ $< common_srv.o -lpthread

recibe_fichero: recibe_fichero.o common_srv.o
	$(CC) -o $@ $< common_srv.o

clean:
	rm -f $(PROGRS) *.o
