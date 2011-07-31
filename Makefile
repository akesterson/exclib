CC=gcc
LD=gcc
EXECOBJ=
LIBOBJECTS=src/exclib.o
DEMOS=demo/single.exe demo/twolevel.exe demo/threelevel.exe demo/trypair.exe
LIBTARGET=lib/libexc.a
LIBS=
CFLAGS=

all: $(LIBTARGET) $(DEMOS)

demo/%.exe: demo/%.o
	$(LD) -o $@ $(CFLAGS) -L./lib $< -lexc -ggdb -gstabs

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) -ggdb -gstabs -I./include $<

$(LIBTARGET): $(LIBOBJECTS)
	ar rcs $(LIBTARGET) $(LIBOBJECTS)

.PHONY: clean
clean:
	rm -f demo/*o $(LIBOBJECTS) $(DEMOS) $(LIBTARGET)
