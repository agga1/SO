CC = gcc -Wall -std=c11 -ggdb
Min ?= 1
Max ?= 1
Nr ?= 1
clean:
	rm -f -r tmp matrixes out lista
	mkdir out/

helper:
	$(CC) helper.c -o out/helper

matrix:
	$(CC) matrix.c -o out/matrix

create_mx: helper
	./out/helper create $(Min) $(Max) $(Nr)

test: clean matrix create_mx
	./out/matrix lista 5 1 joint
#	./out/matrix lista 3 2 disjoint
	./out/helper check matrixes/a0.txt matrixes/b0.txt matrixes/c0.txt