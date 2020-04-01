cc = gcc -Wall -ggdb

all: clean compile

compile:
	$(cc) sender.c -o sender
	$(cc) catcher.c -o catcher

clean:
	rm -f sender catcher

test: all
	@./catcher 5 sigrt
	@./catcher 5 kill
#	./catcher 5 sigqueue
