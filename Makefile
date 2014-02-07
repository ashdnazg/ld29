CC = gcc
CFLAGS = -Wall -g
C_FILES = event.c \
          int_list.c \
          mem_wrap.c \
          macros.c \
          system.c
          
          
HEADER_FILES = event.h \
               int_list.h \
               mem_wrap.h \
               macros.h \
               event_types.h \
               system.h

OBJECT_FILES = $(C_FILES:.c=.o)

$(OBJECT_FILES): $(HEADER_FILES)

all:

test: events_test

events_test.o : $(HEADER_FILES)

events_test: events_test.o $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) events_test.o $(OBJECT_FILES) -o events_test.exe



clean:
	rm -f $(OBJECT_FILES)
	rm -f events_test.o
	rm -f events_test.exe