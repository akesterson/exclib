CC=gcc
LD=gcc
EXECOBJ=
LIBOBJECTS=src/exclib.o
#DEMOS=demo/single.exe demo/twolevel.exe demo/trypair.exe demo/catchgroup.exe demo/finally.exe demo/default.exe demo/helpers.exe demo/deepuncaught.exe demo/cleanup.exe demo/skeleton.exe
DEMOS=demo/skeleton.exe demo/cleanup.exe
LIBTARGET=lib/libexc.a
LIBS=
#CFLAGS=-Wall -Wextra -std=c89 -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
CFLAGS=-std=c89

all: lib demo

demo/%.exe: demo/%.o lib
	$(LD) -o $@ $(CFLAGS) -L./lib $< -lexc -ggdb

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) -ggdb -I./include $<

.PHONY: demo
demo: $(DEMOS)

.PHONY: lib
lib: $(LIBTARGET)

$(LIBTARGET): $(LIBOBJECTS)
	mkdir -p lib
	ar rcs $(LIBTARGET) $(LIBOBJECTS)

.PHONY: clean
clean:
	rm -f demo/*o $(LIBOBJECTS) $(DEMOS) $(LIBTARGET)
