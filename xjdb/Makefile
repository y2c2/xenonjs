CC=gcc
INCLUDE=-I. -I../deps/ec/src
CFLAGS_COMMON=-Wall -Wextra
CFLAGS=$(CFLAGS_COMMON) -g -O0
CFLAGS_RELEASE=$(CFLAGS_COMMON) -O2
LDFLAGS=-L../deps/ec -lec
SRCS=$(wildcard *.c)
TARGET=xjdb
RM=rm -rf

ifeq ($(OS),Windows_NT)
	LDFLAGS+=-lws2_32
else
	LDFLAGS+=-lpthread
endif

default:
	$(CC) $(INCLUDE) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

release:
	$(CC) $(INCLUDE) $(CFLAGS_RELEASE) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	$(RM) $(TARGET)
