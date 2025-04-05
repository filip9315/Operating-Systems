CC = gcc
CFLAGS = -Wall -std=gnu17 -g

all: zad1 zad2

zad1: 
	$(CC) $(CFLAGS) -c zad1.c 
	$(CC) $(CFLAGS) -o zad1 zad1.c 

zad2: 
	$(CC) $(CFLAGS) -c sender.c 
	$(CC) $(CFLAGS) -o sender sender.c
	$(CC) $(CFLAGS) -c catcher.c 
	$(CC) $(CFLAGS) -o catcher catcher.c 

clean:
	rm -f zad1 catcher sender *.o
