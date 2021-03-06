CC = gcc -std=gnu99 -pthread
SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))
all: $(PROGS)
%: %.c
	$(CC) $(CFLAGS)  -o $@ $<

clean:
	rm -f *.o user oss
	echo Clean done
