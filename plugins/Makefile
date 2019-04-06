
CC=gcc
CFLAGS=-Wall -fpic -shared -O2
LIBS=-m
INCLUDE=-I../src

OBJS=plugin_constant.so plugin_ikeda.so plugin_file.so

.PHONY: all clean plugins

all: plugins

clean:
	rm -f $(OBJS)

plugins: $(OBJS)

%.so: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<

