INCLUDES = -I..
CORE_PATH = ../core/
include ../Makefile.common

.PHONY: events_test components_test sdl_test

events_test: events_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) events_test.o $(CORE_O_FILES) -o events_test.exe

components_test: components_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) components_test.o $(CORE_O_FILES) -o components_test.exe

sdl_test: sdl_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) sdl_test.o $(CORE_O_FILES) ../systems/sdl/sdl.o -lmingw32 -lSDL2main -lSDL2 -o sdl_test.exe

    
all: events_test components_test sdl_test

clean:
	rm -f events_test.o
	rm -f events_test.exe
	rm -f components_test.o
	rm -f components_test.exe
	rm -f sdl_test.o
	rm -f sdl_test.exe
