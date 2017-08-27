AR = ar
CC = gcc
CFLAGS = -Wall -ansi -pedantic -O2 -fomit-frame-pointer

OBJECT_FILES = z80emu.o

all: z80emu.a

tables.h: maketables.c
	$(CC) -Wall $< -o maketables
	./maketables > $@

z80emu.o: z80emu.c z80emu.h z80config.h z80user.h \
	instructions.h macros.h tables.h ../zext.h
	$(CC) $(CFLAGS) -c $<

# zextest.o: zextest.c zextest.h z80emu.h z80config.h
# 	$(CC) -Wall -c $<


z80emu.a: $(OBJECT_FILES)
	$(AR) rcs $@ $(OBJECT_FILES)
	# $(CC) $(OBJECT_FILES) -o $@

clean:
	rm -f *.o z80emu.a maketables
