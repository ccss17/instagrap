override CFLAGS += -g -ggdb
CC = gcc
OUTPUT = out/worker

all: worker.c
	$(CC) $? -o $(OUTPUT) $(CFLAGS)
clean:
	rm $(OUTPUT)
#gcc -o out/worker worker.c libworker/util_worker.c -g -ggdb -Ilibworker
