cc = gcc -Wall -ggdb

all: clean compile

compile:
	$(cc) main.c -o main
	$(cc) consumer.c -o consumer
	$(cc) producer.c -o producer


clean:
	rm -f main producer consumer out.txt
	rm -rf fifo

test: all
#	mkfifo fifo
#	./producer fifo files/as 5 &
#	./producer fifo files/bs 5 &
#	./producer fifo files/cs 4 &
#	./consumer fifo out.txt 8
	./main 5 out.txt 4 files/as files/bs files/cs files/ds




