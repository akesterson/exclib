CC=gcc
LD=gcc
EXECOBJ=
LIBOBJECTS=src/exclib.o
DEMOS=demo/single.exe demo/twolevel.exe demo/threelevel.exe demo/trypair.exe demo/catchgroup.exe demo/finally.exe demo/default.exe demo/helpers.exe demo/deepuncaught.exe
LIBTARGET=lib/libexc.a
LIBS=
CFLAGS=

all: lib demo

demo/%.exe: demo/%.o lib
	$(LD) -o $@ $(CFLAGS) -L./lib $< -lexc -ggdb -gstabs

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) -ggdb -gstabs -I./include $<

.PHONY: demo
demo: $(DEMOS)

.PHONY: lib
lib: $(LIBTARGET)

$(LIBTARGET): $(LIBOBJECTS)
	ar rcs $(LIBTARGET) $(LIBOBJECTS)

.PHONY: clean
clean:
	rm -f demo/*o $(LIBOBJECTS) $(DEMOS) $(LIBTARGET)
