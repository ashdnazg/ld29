INCLUDES = -I..
CORE_PATH = ../core/
include ../Makefile.common

events_test: events_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) events_test.o $(CORE_O_FILES) -o events_test.exe

all: events_test

clean:
	rm -f events_test.o
	rm -f events_test.exe
