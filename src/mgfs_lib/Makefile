CFLAGS=-Wall -g -fPIC

all: libmgfs.so

mgfs.o: common.h common_cln.h mgfs.h master.h server.h

libmgfs.so: mgfs.o common.o common_cln.o
	$(CC) $(CFLAGS) -shared -o $@ $< common.o common_cln.o

clean:
	rm -f *.o libmgfs.so


