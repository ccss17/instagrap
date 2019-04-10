override CFLAGS += -c -g -ggdb
CC = gcc
OUTPUT = libworker.a
INCLUDE = ../include

all: libworker
	
libworker: libworker.c
	$(CC) $? -o $(OUTPUT) $(CFLAGS) -I$(INCLUDE)

clean:
	rm $(OUTPUT)
