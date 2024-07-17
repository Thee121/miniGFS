CFLAGS=-Wall -g

all: server

server.o: common.h common_srv.h common_cln.h master.h server.h
common.o: common.h
common_cln.o: common_cln.h
common_srv.o: common_srv.h

server: server.o common.o common_srv.o common_cln.o
	$(CC) -o $@ $< common.o common_srv.o common_cln.o -lpthread

clean:
	rm -f *.o server
