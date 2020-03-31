cc = gcc -Wall -ggdb

all: clean compile

compile:
	$(cc) main.c -o main
	$(cc) test.c -o test
	$(cc) exec_ins.c -o exec_ins

clean:
	rm -f exec_ins test main

test: all
	./main exec ignore only10
	./main fork ignore only10
	./main exec mask only10
	./main fork mask only10
	./main exec pending only10
	./main fork pending only10
	./main fork handler only10