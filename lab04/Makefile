CC = gcc
CFLAGS = -Wall -std=c17 -g

all: zad1 zad2

zad1: 
	$(CC) $(CFLAGS) -c zad1.c
	$(CC) $(CFLAGS) -o zad1 zad1.c 

zad2:
	$(CC) $(CFLAGS) -c zad2.c
	$(CC) $(CFLAGS) -o zad2 zad2.c 

.PHONY: clean

clean:
	rm -f zad1 zad2 *.o