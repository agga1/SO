cc = gcc -Wall -ggdb

all: clean compile

compile:
	$(cc) sender.c -o sender
	$(cc) catcher.c -o catcher

clean:
	rm -f sender catcher

test: all
	@./catcher 50 sigrt
	@./catcher 50 kill
#	./catcher 5 sigqueue
