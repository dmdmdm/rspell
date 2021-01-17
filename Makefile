CC = gcc
LDFLAGS = -lstdc++
CFLAGS = -g -I. -fno-rtti -fno-exceptions -Wall
CPPFLAGS = $(CFLAGS)
SOURCES = dict.cpp dict.h dspell.cpp dspell.h

all: dspell

clean:
	rm -f *.o dspell

format:
	clang-format -style '{BasedOnStyle: Google, DerivePointerBinding: false, Standard: Cpp11}' -i $(SOURCES)

install: all

dspell: dict.o dspell.o
	$(CC) $(CFLAGS) -o $@ dict.o dspell.o $(LDFLAGS)

check: dspell
	echo apple orrange | ./dspell
