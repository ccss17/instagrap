override CFLAGS += -pthread -g -ggdb
CC = gcc
WORKER = bin/worker
SUBMITTER = bin/submitter
INSTAGRAPD = bin/instagrapd
MODULE = instagrap
LIB = lib$(MODULE)

all: lib/$(LIB).a worker submitter instagrapd

lib/$(LIB).a: lib/$(LIB).c
	make -C lib
	
worker: worker.c 
	$(CC) $? $(CFLAGS) -Iinclude -Llib -l$(MODULE) -o $(WORKER) 

submitter: submitter.c
	$(CC) $? $(CFLAGS) -Iinclude -Llib -l$(MODULE) -o $(SUBMITTER) 

instagrapd: instagrapd.c 
	$(CC) $? $(CFLAGS) -Iinclude -Llib -l$(MODULE) -o $(INSTAGRAPD) 

clean:
	make clean -C lib
	rm bin/*
	rm *.out
	rm target.*
