override CFLAGS += -c -g -ggdb
CC = gcc

all: libinstagrap
	
libinstagrap: libinstagrap.c
	$(CC) $? -o libinstagrap.a $(CFLAGS) -I../include

clean:
	rm *.a
