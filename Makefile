override CFLAGS += -g -ggdb
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
	rm $(WORKER)
	rm $(SUBMITTER)
	rm $(INSTAGRAPD)
	rm a.out
	rm target.c
	make clean -C lib
