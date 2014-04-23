.PHONY: tests core systems

core:
	cd core && $(MAKE) all

systems: core
	cd systems && $(MAKE) all

all: core

tests: core systems
	cd tests && $(MAKE) all



clean:
	cd tests && $(MAKE) clean
	cd core && $(MAKE) clean
	cd systems && $(MAKE) clean
