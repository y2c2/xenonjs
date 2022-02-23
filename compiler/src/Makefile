CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -fPIC
CFLAGS1 = 

CC_CHECK = clang
CFLAGS_CHECK = "-Weverything -Wno-padded -Wno-packed -Wno-c++-compat"
MAKE = make

MODE = debug
ifeq ($(MODE),debug)
	CFLAGS += -O0
else ifeq ($(MODE),release)
	CFLAGS += -O2
else ifeq ($(MODE),prof)
	CFLAGS += -pg -O2
endif

BUILDPATH=.build

INCLUDES = -I./ -I../include/ -I../../deps/ec/src/ -I$(BUILDPATH)
RM = rm -rf

SRCS=$(wildcard *.c)
OBJS=$(addprefix $(BUILDPATH)/, $(notdir $(SRCS:.c=.o)))
DEPS=$(OBJS:.o=.d)

default: $(OBJS)

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


