MAKE=make

default:
	$(MAKE) -C deps/ec/
	$(MAKE) -C xjc/
	$(MAKE) -C xjr/
	$(MAKE) -C xjdb/

release:
	$(MAKE) -C deps/ec/ MODE=release
	$(MAKE) -C xjc/ MODE=release
	$(MAKE) -C xjr/ MODE=release
	$(MAKE) -C xjdb/ MODE=release

prof:
	$(MAKE) -C deps/ec/ MODE=prof
	$(MAKE) -C xjc/ MODE=prof
	$(MAKE) -C xjr/ MODE=prof
	$(MAKE) -C xjdb/ MODE=prof

clean:
	$(MAKE) clean -C deps/ec/
	$(MAKE) clean -C xjc/
	$(MAKE) clean -C xjr/
	$(MAKE) clean -C xjdb/
