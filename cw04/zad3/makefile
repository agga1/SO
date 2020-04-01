cc = gcc -Wall -ggdb

all: clean compile

compile:
	$(cc) main.c -o main

clean:
	rm -f main

test: all
	./main 10
	./main 8
	./main 2
	./main 11
