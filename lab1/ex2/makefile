CC = gcc -Wall -std=c11 -ggdb


all: static shared clean

static:
	$(CC) -c library.c
	ar rcs library.a library.o

shared:
	$(CC) -c -fPIC library.c
	$(CC) -shared -fPIC -o library.so library.o

zad2_static:
	make static
	$(CC) -o test main.c library.a -I .
	make tests


clean:
	rm -f *.o

tests:
	# similar files
	./test create_table 5 start_time compare_pairs lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsim.txt
	# kinda similar files
	./test create_table 5 start_time compare_pairs dif.txt:difb.txt dif.txt:difb.txt dif.txt:difb.txt dif.txt:difb.txt dif.txt:difb.txt
	# very different files
	./test create_table 5 start_time compare_pairs lsim.txt:dif.txt lsim.txt:dif.txt lsim.txt:dif.txt lsim.txt:dif.txt lsim.txt:dif.txt
	# a few comparisons
	./test create_table 5 start_time compare_pairs lsim.txt:lsimb.txt
	# some comparisons
	./test create_table 5 start_time compare_pairs lsim.txt:lsimb.txt \
    lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
    lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
    lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt
	# a lot of comparisons
	./test create_table 5 start_time compare_pairs lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt
	# saving small blocks
	./test create_table 5 compare_pairs a.txt:b.txt a.txt:b.txt a.txt:b.txt a.txt:b.txt a.txt:b.txt \
	start_time save_block 0 save_block 1 save_block 2 save_block 3 save_block 4 \
	remove_block 2 remove_block 1
	#saving big blocks
	./test create_table 5 compare_pairs lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt \
	start_time save_block 0 save_block 1 save_block 2 save_block 3 save_block 4
	#removing big blocks
	./test create_table 5 compare_pairs lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt \
	save_block 0 save_block 1 save_block 2 save_block 3 save_block 4\
	start_time remove_block 0 remove_block 1 remove_block 2 remove_block 3 remove_block 4


