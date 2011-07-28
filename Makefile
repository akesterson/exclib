CC=gcc
LD=gcc
EXECOBJ=
OBJECTS=exclib.o exception_test.o
LIBS=
CFLAGS=

all: exception_test

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) -rdynamic -ggdb -gstabs $<

exception_test: $(OBJECTS)
	$(LD) -o exception_test $(LIBS) $(OBJECTS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f exception_test