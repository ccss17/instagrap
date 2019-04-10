# gcc main.cpp hello.cpp factorial.cpp -o hello
override CFLAGS += -O
CC=g++
FILES = main.cpp hello.cpp factorial.cpp
OUTPUT = hello

all: $(FILES) 
	$(CC) $(CFLAGS) $? -o $(OUTPUT)
	#$(CC) $(CFLAGS) $? -o $@
	#$(CC) $(CFLAGS) $@.cpp -o $@

#.PHONY: clean
clean:
	rm $(OUTPUT)
	#touch clean
