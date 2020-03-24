CC = gcc -Wall -std=c11 -ggdb
Min ?= 1
Max ?= 10
Nr ?= 5
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
	./out/matrix lista 6 3 joint
#	./out/matrix lista 6 3 disjoint
	./out/helper check matrixes/a0.txt matrixes/b0.txt matrixes/c0.txt