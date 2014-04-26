.PHONY: test core

DIRS = sdl map

all: core
	@for d in $(DIRS); do (cd $$d; $(MAKE) all ); done

clean:
	@for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done
