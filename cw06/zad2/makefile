CC = gcc -Wall -std=gnu11

all: clean server client

server:
	$(CC) server.c -o server -lrt

client:
	$(CC) client.c -o client -lrt


clean:
	rm -f server client
