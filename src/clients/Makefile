CFLAGS=-Wall -g

all: libmgfs test cp_from_local_to_mGFS

libmgfs:
	$(MAKE) -C ../mgfs_lib

test.o cp_from_local_to_mGFS.o: mgfs.h
test: test.o libmgfs.so
	$(CC) -o $@ $< ./libmgfs.so -Wl,-rpath-link=.

cp_from_local_to_mGFS: cp_from_local_to_mGFS.o libmgfs.so
	$(CC) -o $@ $< ./libmgfs.so -Wl,-rpath-link=.

clean:
	rm -f *.o test cp_from_local_to_mGFS

