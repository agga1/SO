cc = gcc -Wall -ggdb -pthread

all: clean compile

compile:
	$(cc) main.c -o main

clean:
	rm -f main
	rm -rf out/
	mkdir out

test: all
	./main  1 sign mountain.ascii.pgm out/optional_name.ascii.pgm
	./main  2 sign mountain.ascii.pgm
	./main  4 sign mountain.ascii.pgm
	./main  8 sign mountain.ascii.pgm
	./main  1 block mountain.ascii.pgm
	./main  2 block mountain.ascii.pgm
	./main  4 block mountain.ascii.pgm
	./main  8 block mountain.ascii.pgm
	./main  1 interleaved mountain.ascii.pgm
	./main  2 interleaved mountain.ascii.pgm
	./main  4 interleaved mountain.ascii.pgm
	./main  8 interleaved mountain.ascii.pgm


