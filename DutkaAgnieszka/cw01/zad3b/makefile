CC = gcc -Wall -std=c11 -ggdb
Opt ?= 0
all: static shared clean

static:
	$(CC) -c library.c
	ar rcs library.a library.o

shared: clean
	$(CC) -fPIC -shared library.c -o liblibrary.so

zad3_static:
	make static
	$(CC) -o test main.c library.a -I . -O$(Opt)
	make tests

zad3_dynamic:
	make shared
#	$(CC) main_dynamic.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -ldl -o out/main
	$(CC) -o test main_dynamic.c -ldl -I .  -O$(Opt)
	make tests

zad3_shared:
	make shared
	$(CC) main.c -L. -Wl,-rpath=. -llibrary -o test -O$(Opt)
	make tests

clean:
	rm -f *.o
	rm -f tmp_files/*

tests:
	# similar files
	./test clear_file comment similar_files create_table 5 start_time compare_pairs lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsim.txt
	# kinda similar files
	./test comment kinda_similar_files create_table 5 start_time compare_pairs dif.txt:difb.txt dif.txt:difb.txt dif.txt:difb.txt dif.txt:difb.txt dif.txt:difb.txt
	# very different files
	./test comment very_different_files create_table 5 start_time compare_pairs lsim.txt:dif.txt lsim.txt:dif.txt lsim.txt:dif.txt lsim.txt:dif.txt lsim.txt:dif.txt
	# big files comparisons
	./test comment big_files create_table 5 start_time compare_pairs lrg.txt:lrgb.txt lrg.txt:lrgb.txt
	# a few comparisons
	./test comment few_comparisons create_table 5 start_time compare_pairs lsim.txt:lsimb.txt
	# some comparisons
	./test comment more_comparisons create_table 5 start_time compare_pairs lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt
	# a lot of comparisons
	./test comment many_comparisons create_table 5 start_time compare_pairs lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt \
	lsim.txt:lsimb.txt lsim.txt:lsimb.txt lsim.txt:lsimb.txt
	# saving small blocks
	./test comment small_blocks_save create_table 5 compare_pairs small.txt:smallb.txt small.txt:smallb.txt small.txt:smallb.txt small.txt:smallb.txt small.txt:smallb.txt \
	start_time save_block 0 save_block 1 save_block 2 save_block 3 save_block 4 \
	remove_block 2 remove_block 1
	#saving big blocks
	./test comment big_blocks_save create_table 5 compare_pairs lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt \
	start_time save_block 0 save_block 1 save_block 2 save_block 3 save_block 4
	#removing big blocks
	./test comment big_blocks_remove create_table 5 compare_pairs lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt lrgb.txt:lrg.txt \
	save_block 0 save_block 1 save_block 2 save_block 3 save_block 4\
	start_time remove_block 0 remove_block 1 remove_block 2 remove_block 3 remove_block 4

