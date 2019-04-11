override CFLAGS += -g -ggdb
CC = gcc
#OUTPUT = bin/worker
OUTPUT = bin
MODULE = instagrap
LIB = lib$(MODULE)

all: lib/$(LIB).a worker submitter

lib/$(LIB).a: lib/$(LIB).c
	make -C lib
	
worker: worker.c 
	$(CC) $? -o $(OUTPUT)/worker $(CFLAGS) -Iinclude -Llib -l$(MODULE)

submitter: submitter.c
	$(CC) $? -o $(OUTPUT)/submitter $(CFLAGS) -Iinclude -Llib -l$(MODULE)

clean:
	rm $(OUTPUT)
	make clean -C lib
