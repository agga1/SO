CC = gcc -Wall -std=c11 -g

all: clean main creator packer sender
	./main

main:
	$(CC) main.c -o main

creator:
	$(CC) creator.c -o creator

packer:
	$(CC) packer.c -o packer

sender:
	$(CC) sender.c -o sender

clean:
	rm -f main creator packer sender