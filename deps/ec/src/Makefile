CC=gcc
CFLAGS=-Wall -Wextra -MMD -fPIC
CFLAGS1=

CC_CHECK=clang
CFLAGS_CHECK="-Weverything -Wno-padded -Wno-packed"
MAKE=make

MODE=debug
ifeq ($(MODE),debug)
	CFLAGS+=-O0 -g
else ifeq ($(MODE),release)
	CFLAGS+=-O2
else ifeq ($(MODE),prof)
	CFLAGS+=-O2
endif

INCLUDES=-I./
RM=rm -rf

BUILDPATH=.build
SRCS=$(wildcard *.c)
OBJS=$(addprefix $(BUILDPATH)/, $(notdir $(SRCS:.c=.o)))
DEPS=$(OBJS:.o=.d)

default: $(OBJS)

release:
	$(MAKE) MODE=release

prof:
	$(MAKE) MODE=prof

$(BUILDPATH)/%.o: %.c
	@-mkdir -p $(BUILDPATH)
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES) -o $@ -c $<

all: clean $(OBJS)

clean:
	@$(RM) $(BUILDPATH)/*.o $(BUILDPATH)/*.d
	@$(RM) $(BUILDPATH)

-include $(DEPS)

check:
	@$(MAKE) CC=$(CC_CHECK) CFLAGS1=$(CFLAGS_CHECK)

