.PHONY: tests core systems external main
ROOT_PATH = .
LIBS = $(join  $(SYSTEMS:%=systems/%/), $(SYSTEMS:%=%.a))
include Makefile.common

main: core systems external main.o
	$(CC) $(CFLAGS) $(LDFLAGS) main.o $(CORE_O_FILES) $(EXTERNAL_O_FILES) $(LIBS) -lmingw32 -lSDL2main -lSDL2 -o ld29.exe

core: external
	cd core && $(MAKE) all

systems: core
	cd systems && $(MAKE) all

all: main

tests: core systems
	cd tests && $(MAKE) all

external:
	cd external && $(MAKE) all

clean:
	cd tests && $(MAKE) clean
	cd core && $(MAKE) clean
	cd systems && $(MAKE) clean
	rm -f ld29.exe

clean-external:
	cd external && $(MAKE) clean
    