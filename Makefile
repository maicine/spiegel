CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

spiegel: $(OBJS)
	gcc -o $@ $^

$(OBJS): spiegel.h

test: spiegel
		./spiegel -test
		./test.sh

clean: 
		rm -f spiegel *.o *~ tmp*