CFLAGS=-Wall -g -fPIC

all: libutil.so demo_map_con_array_interno demo_solo_array

map.o: map.h
demo_solo_array array.o: array.h
demo_map_con_array_interno.o: map.h array.h

libutil.so: map.o array.o
	$(CC) $(CFLAGS) -shared -o $@ $^ -lpthread

demo_map_con_array_interno: demo_map_con_array_interno.o libutil.so
	$(CC) -o $@ $< ./libutil.so -lpthread

demo_solo_array: demo_solo_array.o libutil.so
	$(CC) -o $@ $< ./libutil.so -lpthread

clean:
	rm -f *.o libutil.so demo_map_con_array_interno demo_solo_array
