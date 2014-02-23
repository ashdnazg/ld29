CC = gcc
CFLAGS = -pedantic -std=c99 -Wall -g
TEST_LIB = libaesis.a
C_FILES = event.c \
          int_list.c \
          mem_wrap.c \
          macros.c \
          system.c \
          
          
HEADER_FILES = event.h \
               int_list.h \
               mem_wrap.h \
               macros.h \
               builtin_events.h \
               system.h \

OBJECT_FILES = $(C_FILES:.c=.o)
    
all:
	@echo nothing here at the moment

.PHONY: all test

$(OBJECT_FILES): $(HEADER_FILES)


test: $(OBJECT_FILES) $(TEST_LIB)
	cd tests && $(MAKE)

$(TEST_LIB): $(OBJECT_FILES)
	ar rcs $(TEST_LIB) $(OBJECT_FILES)



clean:
	rm -f $(OBJECT_FILES)
	rm -f $(TEST_LIB)
	cd tests && $(MAKE) clean
    
