cc = gcc -Wall -ggdb -pthread -Wno-missing-braces

all: clean client server

client:
	$(cc) client.c -o client

server:
	$(cc) server.c -o server

clean:
	rm -f client server

test_s: all
	./server 1234 /tmp/socket

test_c1:
	./client nick1 local /tmp/socket

test_c2:
	./client nick2 local /tmp/socket

test_c1n:
	./client nick1 net 1234

test_c2n:
	./client nick2 net 1234