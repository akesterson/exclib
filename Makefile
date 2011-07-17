CC=gcc
LD=gcc
OBJECTS=exclib.o exception_test.o

all: exception_test

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) -rdynamic -ggdb -gstabs $<

exception_test: $(OBJECTS)
	$(LD) -o exception_test $(OBJECTS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f exception_test