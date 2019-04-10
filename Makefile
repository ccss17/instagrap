override CFLAGS += -g -ggdb
CC = gcc
OUTPUT = out/worker

all: lib/libworker.a worker 

lib/libworker.a: lib/libworker.c
	make -C lib
	
worker: worker.c 
	$(CC) $? -o $(OUTPUT) $(CFLAGS) -Iinclude -Llib -lworker

clean:
	rm $(OUTPUT)
	make clean -C lib
