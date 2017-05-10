CC = gcc
CFLAGS = -Iheaders -Wall -Wextra -std=c99

all:
	$(CC) $(CFLAGS) stack.c fifth.c hex.c hash.c intern.c table.c -o fifth
	$(CC) $(CFLAGS) hash_util.c hash.c -o hash_util

