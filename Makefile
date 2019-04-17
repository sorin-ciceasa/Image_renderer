all: filter

filter: filter.c filter.h
	gcc -o filter filter.c main.c -lpthread -Wall -lm

.PHONY: clean
clean:
	rm -f filter
