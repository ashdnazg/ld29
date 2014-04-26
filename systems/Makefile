.PHONY: test core
ROOT_PATH = ..

include ../Makefile.common

all: core
	@for d in $(SYSTEMS); do (cd $$d; $(MAKE) all ); done

clean:
	@for d in $(SYSTEMS); do (cd $$d; $(MAKE) clean ); done
