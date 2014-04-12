INCLUDES = -I..
include ../Makefile.common

events_test: events_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) events_test.o ../core/$(CORE_LIB) -o events_test.exe

all: events_test

clean:
	rm -f events_test.o
	rm -f events_test.exe
