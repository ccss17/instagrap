override CFLAGS += -c -g -ggdb
CC = gcc
MODULE = instagrap
LIB = lib$(MODULE)
OUTPUT = libinstagrap.a
INCLUDE = ../include

all: $(LIB)
	
$(LIB): $(LIB).c
	$(CC) $? -o $(OUTPUT) $(CFLAGS) -I$(INCLUDE)

clean:
	rm $(OUTPUT)
