CC = gcc -Wall -std=c11 -ggdb
all: static shared clean

compile: clean
	$(CC) -o out/main main.c

clean:
	rm -rf out/
	mkdir out/

tests:
	make compile
	./out/main comment --gen-1 generate test1 80000 1
	./out/main comment copy-lib copy test1 test1s 40000 1 lib
	./out/main comment sort-lib sort test1s 40000 1 lib
	./out/main comment copy-sys copy test1 test1s 40000 1 sys
	./out/main comment sort-sys sort test1s 40000 1 lib
	./out/main comment copy-lib copy test1 test1s 70000 1 lib
	./out/main comment sort-lib sort test1s 70000 1 lib
	./out/main comment copy-sys copy test1 test1s 70000 1 sys
	./out/main comment sort-sys sort test1s 70000 1 lib
	./out/main comment --gen-4 generate test2 80000 4
	./out/main comment copy-lib copy test2 test2s 40000 4 lib
	./out/main comment sort-lib sort test2s 40000 4 lib
	./out/main comment copy-sys copy test2 test2s 40000 4 sys
	./out/main comment sort-sys sort test2s 40000 4 sys
	./out/main comment copy-lib copy test2 test2s 70000 4 lib
	./out/main comment sort-lib sort test2s 70000 4 lib
	./out/main comment copy-sys copy test2 test2s 70000 4 sys
	./out/main comment sort-sys sort test2s 70000 4 sys
	./out/main comment --gen-512 generate test3 80000 512
	./out/main comment copy-lib copy test3 test3s 40000 512 lib
	./out/main comment sort-lib sort test3s 40000 512 lib
	./out/main comment copy-sys copy test3 test3s 40000 512 sys
	./out/main comment sort-sys sort test3s 40000 512 sys
	./out/main comment copy-lib copy test3 test3s 70000 512 lib
	./out/main comment sort-lib sort test3s 70000 512 lib
	./out/main comment copy-sys copy test3 test3s 70000 512 sys
	./out/main comment sort-sys sort test3s 70000 512 sys
	./out/main comment --gen-1024 generate test4 80000 1024
	./out/main comment copy-lib copy test4 test4s 40000 1024 lib
	./out/main comment sort-lib sort test4s 40000 1024 lib
	./out/main comment copy-sys copy test4 test4s 40000 1024 sys
	./out/main comment sort-sys sort test4s 40000 1024 sys
	./out/main comment copy-lib copy test4 test4s 70000 1024 lib
	./out/main comment sort-lib sort test4s 70000 1024 lib
	./out/main comment copy-sys copy test4 test4s 70000 1024 sys
	./out/main comment sort-sys sort test4s 70000 1024 sys
	./out/main comment --gen-4096 generate test5 80000 4096
	./out/main comment copy-lib copy test5 test5s 40000 4096 lib
	./out/main comment sort-lib sort test5s 40000 4096 lib
	./out/main comment copy-sys copy test5 test5s 40000 4096 sys
	./out/main comment sort-sys sort test5s 40000 4096 sys
	./out/main comment copy-lib copy test5 test5s 70000 4096 lib
	./out/main comment sort-lib sort test5s 70000 4096 lib
	./out/main comment copy-sys copy test5 test5s 70000 4096 sys
	./out/main comment sort-sys sort test5s 70000 4096 sys
	./out/main comment --gen-8192 generate test6 80000 8192
	./out/main comment copy-lib copy test6 test6s 40000 8192 lib
	./out/main comment sort-lib sort test6s 40000 8192 lib
	./out/main comment copy-sys copy test6 test6s 40000 8192 sys
	./out/main comment sort-sys sort test6s 40000 8192 sys
	./out/main comment copy-lib copy test6 test6s 70000 8192 lib
	./out/main comment sort-lib sort test6s 70000 8192 lib
	./out/main comment copy-sys copy test6 test6s 70000 8192 sys
	./out/main comment sort-sys sort test6s 70000 8192 sys

