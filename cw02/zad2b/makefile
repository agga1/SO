CC = gcc -Wall -std=gnu99 -ggdb
all: clean compile

compile:
	$(CC) -o out/main main.c
	make tests

clean:
	rm -rf out/
	mkdir out/

tests:
	./out/main . -mtime -30
	./out/main /usr -atime -7 -maxdepth 2
	./out/main /etc -atime -7 -maxdepth 3