override CFLAGS += -g -ggdb
CC = gcc
WORKER = bin/worker
SUBMITTER = bin/submitter
MODULE = instagrap
LIB = lib$(MODULE)

all: lib/$(LIB).a worker submitter

lib/$(LIB).a: lib/$(LIB).c
	make -C lib
	
worker: worker.c 
	$(CC) $? -o $(WORKER) $(CFLAGS) -Iinclude -Llib -l$(MODULE)

submitter: submitter.c
	$(CC) $? -o $(SUBMITTER) $(CFLAGS) -Iinclude -Llib -l$(MODULE)

clean:
	rm $(WORKER)
	rm $(SUBMITTER)
	rm a.out
	rm target.c
	make clean -C lib
