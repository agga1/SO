CC = gcc -Wall -std=c11 -ggdb
Min ?= 5
Max ?= 5
Nr ?= 6
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
#	./out/matrix lista 10 5 joint 20 20
	./out/matrix lista 10 2 disjoint 20 20
	./out/helper check matrixes/a0.txt matrixes/b0.txt matrixes/c0.txt