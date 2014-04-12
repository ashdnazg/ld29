.PHONY: test core

core:
	cd core && $(MAKE) all

all: core
	@echo nothing here at the moment


test: core
	cd tests && $(MAKE) all



clean:
	cd tests && $(MAKE) clean
	cd core && $(MAKE) clean
