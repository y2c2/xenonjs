CC=gcc
INCLUDE=-I../rts/include -I../rts/src -I./lib -I./main
CFLAGS=-Wall -Wextra
CFLAGS_DEBUG=-g -O0
CFLAGS_RELEASE=-O2
CFLAGS_PROF=-no-pie -pg -O2
LDFLAGS=-lm
SRCS=$(wildcard ../rts/src/*.c) $(wildcard ./lib/*.c) $(wildcard ./main/*.c)
TARGET=xjr
MAKE=make

MODE=debug
ifeq ($(MODE),debug)
	CFLAGS+=$(CFLAGS_DEBUG)
else ifeq ($(MODE),release)
	CFLAGS+=$(CFLAGS_RELEASE)
else ifeq ($(MODE),prof)
	CFLAGS+=$(CFLAGS_PROF)
endif

ifeq ($(OS),Windows_NT)
	TARGET=xjr.exe
	LDFLAGS=-static
	LDFLAGS+=-lws2_32
else
	TARGET=xjr
endif

default:
	$(CC) $(INCLUDE) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

release:
	$(MAKE) MODE=release

prof:
	$(MAKE) MODE=prof

clean:
	rm -rf $(TARGET)
