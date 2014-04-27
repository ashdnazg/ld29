ROOT_PATH = ../..

include ../../Makefile.common


SYSTEM_NAME = text
LIB_NAME = $(SYSTEM_NAME).a
HEADERS = text.h
C_FILES = text.c
O_FILES = $(C_FILES:%.c=%.o)

$(O_FILES): $(HEADERS) 

$(LIB_NAME): $(O_FILES) 
	ar rcs $(LIB_NAME) $(O_FILES) 

all: $(LIB_NAME)


clean:
	rm -f $(O_FILES) $(LIB_NAME) 