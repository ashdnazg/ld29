ROOT_PATH = ../..

include ../../Makefile.common


SYSTEM_NAME = commands
LIB_NAME = $(SYSTEM_NAME).a
HEADERS = commands.h
C_FILES = commands.c
O_FILES = $(C_FILES:%.c=%.o)

$(O_FILES): $(HEADERS) 

$(LIB_NAME): $(O_FILES) 
	ar rcs $(LIB_NAME) $(O_FILES) 

all: $(LIB_NAME)


clean:
	rm -f $(O_FILES) $(LIB_NAME) 