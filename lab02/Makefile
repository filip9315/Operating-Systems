CC = gcc
CFLAGS = -Wall -std=c17 -g

all_static: collatz_static main_static

all_shared: collatz_shared main_shared

all_dynamic: collatz_shared main_dynamic

main_static:
	$(CC) $(CFLAGS) -c main.c 
	$(CC) $(CFLAGS) main.o collatz.a -o main

main_shared:
	$(CC) $(CFLAGS) main.c -lcollatz -L ./ -Wl,-rpath,. -o main 

main_dynamic:
	$(CC) $(CFLAGS) main_dynamic.c -o main_dynamic

collatz_static: 
	$(CC) $(CFLAGS) -c collatz.c
	ar rcs collatz.a collatz.o

collatz_shared:
	$(CC) -fPIC -fvisibility=default -c collatz.c 
	$(CC) -shared -Wl,-install_name,libcollatz.so.1 \
		-o libcollatz.so.1.0.1 collatz.o -lc 
	ln -sf libcollatz.so.1.0.1 libcollatz.so.1
	ln -sf libcollatz.so.1 libcollatz.so

.PHONY: clean

clean:
	rm -f *.a *.o *.so.* *.so main main_dynamic
