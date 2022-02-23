MAKE=make
AR=ar
AR_FLAGS=rcs
CP=cp
RM=rm -rf
MKDIR=mkdir -p
LIBNAME=ec
TARGET=lib$(LIBNAME).a
SRCS=$(wildcard ./src/*.c)
HDRS=$(wildcard ./src/*.h)
OBJS=$(patsubst ./src/%.c,./src/.build/%.o,$(wildcard ./src/*.c))
MODE=debug

ifeq ($(OS),Windows_NT)
else
	UNAME_S:=$(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		AR=libtool
		AR_FLAGS=-static
	endif
endif

default:
	$(MAKE) -C ./src MODE=$(MODE)
	$(AR) $(AR_FLAGS) -o $(TARGET) $(OBJS)

release:
	$(MAKE) -C ./src MODE=release
	$(AR) $(AR_FLAGS) -o $(TARGET) $(OBJS)

debug:
	$(MAKE) -C ./src MODE=debug
	$(AR) $(AR_FLAGS) -o $(TARGET) $(OBJS)

prof:
	$(MAKE) -C ./src MODE=prof
	$(AR) $(AR_FLAGS) -o $(TARGET) $(OBJS)

clean:
	$(MAKE) -C ./src clean
	$(RM) $(TARGET)

install:
	$(CP) $(TARGET) /usr/lib/
	$(MKDIR) /usr/include/ec/
	$(CP) ./src/*.h /usr/include/ec

uninstall:
	$(RM) /usr/lib/$(TARGET)
	$(RM) /usr/include/ec/

check:
	$(MAKE) -C ./src check

format:
	clang-format -i src/*.c src/*.h
	clang-format -i test/*.c test/*.h
