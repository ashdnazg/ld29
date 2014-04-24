ROOT_PATH = ../..

include ../../Makefile.common


SYSTEM_NAME = sdl
LIB_NAME = $(SYSTEM_NAME).a
HEADERS = sdl.h sdl_video.h sdl_audio.h
C_FILES = sdl.c sdl_video.c sdl_audio.c
O_FILES = $(C_FILES:%.c=%.o)

$(O_FILES): $(HEADERS) 

$(LIB_NAME): $(O_FILES) 
	ar rcs $(LIB_NAME) $(O_FILES) 

all: $(LIB_NAME)


clean:
	rm -f $(O_FILES) $(LIB_NAME) 