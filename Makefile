CC = gcc
LDFLAGS = -lstdc++
CFLAGS = -g -I. -fno-rtti -fno-exceptions -Wall
CPPFLAGS = $(CFLAGS)
SOURCES = dict.cpp dict.h rspell.cpp rspell.h

all: rspell

clean:
	rm -f *.o rspell

format:
	clang-format -style '{BasedOnStyle: Google, DerivePointerBinding: false, Standard: Cpp11}' -i $(SOURCES)

install: all

rspell: dict.o rspell.o
	$(CC) $(CFLAGS) -o $@ dict.o rspell.o $(LDFLAGS)

check: rspell
	echo apple orrange | ./rspell
