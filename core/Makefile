INCLUDES = -I..
include ../Makefile.common

$(CORE_O_FILES): $(CORE_HEADERS)

$(CORE_LIB): $(CORE_O_FILES)
	ar rcs $(CORE_LIB) $(CORE_O_FILES)

all: $(CORE_LIB)


clean:
	rm -f $(CORE_O_FILES) $(CORE_LIB)
