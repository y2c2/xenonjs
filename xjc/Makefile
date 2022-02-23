CC=gcc
RM=rm -rf
CFLAGS=-Wall -Wextra -MMD -fPIC
CFLAGS_DEBUG=-g -O0
CFLAGS_RELEASE=-O2
CFLAGS_PROF=-pg -O2 --no-pie
CFLAGS1=

CC_CHECK=clang
CFLAGS_CHECK="-Weverything -Wno-padded -Wno-packed -Wno-format-nonliteral"
MAKE = make

MODE=debug
ifeq ($(MODE),debug)
	CFLAGS+=$(CFLAGS_DEBUG)
else ifeq ($(MODE),release)
	CFLAGS+=$(CFLAGS_RELEASE)
else ifeq ($(MODE),prof)
	CFLAGS+=$(CFLAGS_RELEASE)
endif

BUILDPATH=.build
BUILDPATH_LIB=.build_lib

INCLUDES=-I./ -I../compiler/include -I../deps/ec/src
LDFLAGS=-L../deps/ec -lec

SRCS=$(wildcard *.c)
OBJS=$(addprefix $(BUILDPATH)/, $(notdir $(SRCS:.c=.o)))
DEPS=$(OBJS:.o=.d)

SRCS_LIB=$(wildcard ../compiler/src/*.c)
OBJS_LIB=$(addprefix $(BUILDPATH_LIB)/, $(notdir $(SRCS_LIB:.c=.o)))
DEPS_LIB=$(OBJS_LIB:.o=.d)

TARGET=xjc

default: xjc_part lib_part
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES) -o $(TARGET) $(OBJS) $(OBJS_LIB) $(LDFLAGS)

release:
	$(MAKE) MODE=release

prof:
	$(MAKE) MODE=prof

xjc_part: $(OBJS)

lib_part: $(OBJS_LIB)

$(BUILDPATH)/%.o: %.c
	@-mkdir -p $(BUILDPATH)
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES) -o $@ -c $<

$(BUILDPATH_LIB)/%.o: ../compiler/src/%.c
	@-mkdir -p $(BUILDPATH_LIB)
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES) -o $@ -c $<

all: clean $(OBJS)

clean:
	@$(RM) $(BUILDPATH)/*.o $(BUILDPATH)/*.d
	@$(RM) $(BUILDPATH_LIB)/*.o $(BUILDPATH_LIB)/*.d
	@$(RM) $(BUILDPATH)
	@$(RM) $(BUILDPATH_LIB)
	@$(RM) $(TARGET)

-include $(DEPS)

check:
	@$(MAKE) CC=$(CC_CHECK) CFLAGS1=$(CFLAGS_CHECK)

