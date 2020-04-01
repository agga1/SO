cc = gcc -Wall -ggdb

all: clean compile

compile:
	$(cc) main.c -o main

clean:
	rm -f main

test: all
	./main sigalarm
	./main sigchld
	./main sigusr1
