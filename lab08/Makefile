CC = gcc
CLFLAGS = -Wall -std=c17

all: main

client:
	$(CC) $(CLFLAGS) -c main.c
	$(CC) $(CLFLAGS) -o main main.c
	
clean:
	rm -f main *.o
