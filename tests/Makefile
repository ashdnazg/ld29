CC = gcc
CFLAGS = -pedantic -std=c99 -Wall -g -I../
TEST_LIB = libaesis.a

    
all: events_test

.PHONY: all

events_test: events_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) events_test.o ../$(TEST_LIB) -o events_test.exe



clean:
	rm -f events_test.o
	rm -f events_test.exe
    
