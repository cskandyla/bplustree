CC=gcc
CCFLAGS=-std=c11 -g -ggdb
LDFLAGS=-lm
ABTREESOURCE= src/hashtable.c src/lru.c src/ab-tree.c src/main.c
OUTPUTFILE= abtree


abtree:
		$(CC) $(CCFLAGS) $(ABTREESOURCE) -o $(OUTPUTFILE)

clean:
		rm -f $(OUTPUTFILE)
