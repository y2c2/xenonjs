CC=gcc
INCLUDE=-I../../rts/include -I../../rts/src -I../lib
CFLAGS_COMMON=-fsyntax-only -Wall -Wextra
CFLAGS=$(CFLAGS_COMMON) -g -O0
CFLAGS_RELEASE=$(CFLAGS_COMMON) -O2
LDFLAGS=
SRCS=$(wildcard *.c) $(wildcard ../../rts/src/*.c) $(wildcard ../lib/*.c)
TARGET=xjr
RM=rm -rf

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
	$(CC) $(INCLUDE) $(CFLAGS_RELEASE) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	$(RM) $(TARGET)
