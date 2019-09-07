.PHONY:all clean
all : odbmain.out
OPT=-g -O0

bpmain.out : bpmain.c bptree.c bptree.h
	gcc $(OPT) -o bpmain.out bpmain.c bptree.c
bemain.out : bemain.c betree.c betree.h
	gcc $(OPT) -o bemain.out bemain.c betree.c
odbmain.out : odbmain.c odb.c odb.h bptree.c bptree.h xxhash.c xxhash.h xxh3.h
	gcc $(OPT) -o odbmain.out odbmain.c odb.c bptree.c xxhash.c

clean :
	rm -f *.out
