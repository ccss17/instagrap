override CFLAGS += -c -g -ggdb
CC = gcc

all: libinstagrap
	
libinstagrap: libinstagrap.c
	$(CC) $? -o libinstagrap.a $(CFLAGS) -I../include

clean:
	if test -d libinstagrap.a; then rm libinstagrap.a/*; fi
