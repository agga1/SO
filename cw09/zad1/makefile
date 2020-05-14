cc = gcc -Wall -ggdb -pthread

all: clean compile

compile:
	$(cc) main.c -o main

clean:
	rm -f main

test: all
	./main  2 6


