override CFLAGS += -g -ggdb
CC = gcc
OUTPUT = bin/worker
MODULE = instagrap
LIB = lib$(MODULE)

all: lib/$(LIB).a worker 

lib/$(LIB).a: lib/$(LIB).c
	make -C lib
	
worker: worker.c 
	$(CC) $? -o $(OUTPUT) $(CFLAGS) -Iinclude -Llib -l$(MODULE)

clean:
	rm $(OUTPUT)
	make clean -C lib
