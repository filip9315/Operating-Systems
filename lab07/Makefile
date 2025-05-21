CC = gcc
CLFLAGS = -Wall -std=c17

all: client server

client:
	$(CC) $(CLFLAGS) -c client.c
	$(CC) $(CLFLAGS) -o client client.c
	

server:
	$(CC) $(CLFLAGS) -c server.c
	$(CC) $(CLFLAGS) -o server server.c
	
clean:
	rm -f client server *.o
