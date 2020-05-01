CC = gcc -Wall -std=c11 -g

all: clean main creator packer sender
	./main

main:
	$(CC) main.c -o main -lrt -lpthread

creator:
	$(CC) creator.c -o creator -lrt -lpthread

packer:
	$(CC) packer.c -o packer -lrt -lpthread

sender:
	$(CC) sender.c -o sender -lrt -lpthread

clean:
	rm -f main creator packer sender