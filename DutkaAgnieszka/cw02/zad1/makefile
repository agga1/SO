CC = gcc -Wall -std=c11 -ggdb
all: static shared clean

compile: clean
	$(CC) -o out/main main.c

clean:
	rm -rf out/
	mkdir out/

tests:
	make compile
	./out/main generate test1 10 112
	./out/main copy test1 test2 10 112 sys
	./out/main copy test1 test3 10 112 lib
#	./out/main copy test3 test4 10 512 sys
#	./out/main sort test1 10 512 lib

