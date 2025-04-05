CC = gcc
CFLAGS = -Wall -std=c17 -g

all: countdown

countdown:
	$(CC) $(CFLAGS) -c countdown.c
	$(CC) $(CFLAGS) -o countdown countdown.o

clean:
	rm countdown countdown.o

.PHONY: clean
