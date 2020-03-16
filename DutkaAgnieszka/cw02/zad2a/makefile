CC = gcc -Wall -std=gnu99 -ggdb
Opt ?= 0
all: clean compile

compile:
	$(CC) -o out/main main.c
	make tests

clean:
	rm -rf out/
	mkdir out/

tests:
#	./out/main .
	./out/main . -mtime +30
	./out/main . -mtime -30
	./out/main . -mtime 30 -atime 30