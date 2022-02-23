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

INCLUDES_COMPILER=-I../compiler/include -I../deps/ec/src
INCLUDES_RTS=-I../rts/include -I../rts/src
INCLUDES=-I./ $(INCLUDES_COMPILER) $(INCLUDES_RTS)
LDFLAGS=-L../deps/ec -lec -lm

BUILDPATH=.build
SRCS=$(wildcard *.c)
OBJS=$(addprefix $(BUILDPATH)/, $(notdir $(SRCS:.c=.o)))
DEPS=$(OBJS:.o=.d)

BUILDPATH_LIB_COMPILER=.build_lib_compiler
SRCS_LIB_COMPILER=$(wildcard ../compiler/src/*.c)
OBJS_LIB_COMPILER=$(addprefix $(BUILDPATH_LIB_COMPILER)/, $(notdir $(SRCS_LIB_COMPILER:.c=.o)))
DEPS_LIB_COMPILER=$(OBJS_LIB_COMPILER:.o=.d)

BUILDPATH_LIB_RTS=.build_lib_rts
SRCS_LIB_RTS=$(wildcard ../rts/src/*.c)
OBJS_LIB_RTS=$(addprefix $(BUILDPATH_LIB_RTS)/, $(notdir $(SRCS_LIB_RTS:.c=.o)))
DEPS_LIB_RTS=$(OBJS_LIB_RTS:.o=.d)

TARGET = xjs

default: xjs_part lib_part_compiler lib_part_rts
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES) -o $(TARGET) $(OBJS) $(OBJS_LIB_COMPILER) $(OBJS_LIB_RTS) $(LDFLAGS)

release:
	$(MAKE) MODE=release

prof:
	$(MAKE) MODE=prof

xjs_part: $(OBJS) $(OBJS_LIB_COMPILER) $(OBJS_LIB_RTS)

lib_part_compiler: $(OBJS_LIB_COMPILER)

lib_part_rts: $(OBJS_LIB_RTS)

$(BUILDPATH)/%.o: %.c
	@-mkdir -p $(BUILDPATH)
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES) -o $@ -c $<

$(BUILDPATH_LIB_COMPILER)/%.o: ../compiler/src/%.c
	@-mkdir -p $(BUILDPATH_LIB_COMPILER)
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES_COMPILER) -o $@ -c $<

$(BUILDPATH_LIB_RTS)/%.o: ../rts/src/%.c
	@-mkdir -p $(BUILDPATH_LIB_RTS)
	$(CC) $(CFLAGS) $(CFLAGS1) $(INCLUDES_RTS) -o $@ -c $<

all: clean $(OBJS)

clean:
	@$(RM) $(BUILDPATH)/*.o $(BUILDPATH)/*.d
	@$(RM) $(BUILDPATH_LIB_COMPILER)/*.o $(BUILDPATH_LIB_COMPILER)/*.d
	@$(RM) $(BUILDPATH_LIB_RTS)/*.o $(BUILDPATH_LIB_RTS)/*.d
	@$(RM) $(BUILDPATH)
	@$(RM) $(BUILDPATH_LIB_COMPILER)
	@$(RM) $(BUILDPATH_LIB_RTS)
	@$(RM) $(TARGET)

-include $(DEPS)

check:
	@$(MAKE) CC=$(CC_CHECK) CFLAGS1=$(CFLAGS_CHECK)

