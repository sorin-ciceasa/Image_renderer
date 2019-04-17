all: filter

filter: filter.h filter.c
	gcc -o filter filter.c main.c -lpthread -Wall -lm

.PHONY: clean
clean:
	rm -f filter
