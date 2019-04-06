
CC=gcc
CFLAGS=-Wall -O2
LDFLAGS=-Xlinker -rpath -Xlinker ./plugins
LIBS=-lm -ldl -lSDL -lSDL_ttf

PROG=anynome
OBJS=parse.o dict.o sound.o stimulus.o anynome.o plugin.o

.PHONY: all clean

all: $(PROG)

clean:
	rm -f $(OBJS) $(PROG) core

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) $(OBJS) -o $(PROG)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

