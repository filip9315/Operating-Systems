CC = gcc
CFLAGS = -Wall -std=c17 -g

all: flipper

flipper:
	$(CC) $(CFLAGS) -c flipper.c 
	$(CC) $(CFLAGS) -o flipper flipper.c 

.PHONY: clean

clean:
	rm -f flipper *.o