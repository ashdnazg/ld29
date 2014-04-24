.PHONY: tests core systems external

core: external
	cd core && $(MAKE) all

systems: core
	cd systems && $(MAKE) all

all: core

tests: core systems
	cd tests && $(MAKE) all

external:
	cd external && $(MAKE) all

clean:
	cd tests && $(MAKE) clean
	cd core && $(MAKE) clean
	cd systems && $(MAKE) clean

clean-external:
	cd external && $(MAKE) clean
    