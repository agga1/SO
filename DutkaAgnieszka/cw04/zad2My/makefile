cc = gcc -Wall -ggdb

all: clean compile

compile:
	$(cc) main.c -o main
	$(cc) test.c -o test
	$(cc) exec_ins.c -o exec_ins

clean:
	rm -f exec_ins test main

test: all
	./main exec ignore
	./main fork ignore
	./main exec mask
	./main fork mask
	./main exec pending
	./main fork pending
	./main fork handler