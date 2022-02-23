MAKE=make
SUBDIRS := $(sort $(wildcard */.))
MODE=

all: $(SUBDIRS)

test:
	make MODE=test

clean:
	make MODE=clean

$(SUBDIRS):
	@$(MAKE) -C $@ $(MODE)

.PHONY: all $(SUBDIRS)
