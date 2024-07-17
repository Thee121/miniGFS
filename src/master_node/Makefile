CFLAGS=-Wall -g -Iutil

all: libutil master

libutil:
	$(MAKE) -C util

master.o: common.h common_srv.h master.h
common.o: common.h
common_srv.o: common_srv.h

master: master.o common.o common_srv.o libutil.so
	$(CC) -o $@ $< common.o common_srv.o -lpthread ./libutil.so

clean:
	rm -f *.o master
