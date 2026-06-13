# Makefile
# vim: set ft=make tw=72 nowrap:

.POSIX:

## Directories
PWD   = $(CURDIR)
I_DIR = $(PWD)/include
S_DIR = $(PWD)/src
L_DIR = $(PWD)/lib
O_DIR = $(PWD)/obj
B_DIR = $(PWD)/bin

SHELL=/bin/bash

## Compiler & linker opts.
CC         = clang # gcc, clang
CCSTD      = c99 # c89 | c90, c99, c11, c17, c20, c2x
CCOPT      = 3
CCOPTS     = -pedantic -pedantic-errors
CCEXTRA    = -fdiagnostics-color=always -fdiagnostics-show-location=once \
             -Wno-padded -Wno-cast-align
CCWARN     = -Wpedantic -Wall -Wshadow -Wextra -Wwrite-strings \
             -Wconversion -Werror
CCFLAGS    = $(CCOPTS) $(CCWARN) -std=$(CCSTD) $(CCEXTRA) -I $(I_DIR)
LDFLAGS    = -lX11 -lXpm -L $(L_DIR) -s

# Use `make DEBUG=1` to add debugging information, symbol table, etc.
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CCFLAGS += -DDEBUG -g -ggdb -O0
else ifeq ($(DEBUG), 2)
	CCFLAGS += -DDEBUG -g -ggdb -O0
	LDFLAGS += -fsanitize=address -fno-omit-frame-pointer -fPIC
else
	CCFLAGS += -DNDEBUG -O$(CCOPT)
endif


## Makefile opts.
SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .h .c .o


## Files options
TARGET = $(B_DIR)/main
OBJS = $(patsubst $(S_DIR)/%.c, $(O_DIR)/%.o, $(wildcard $(S_DIR)/*.c))
ARGS =


## Make options
all: mkdirs $(TARGET)

mkdirs:
	@mkdir -p $(B_DIR) $(O_DIR)
	@for dir in $$(find $(S_DIR) -mindepth 1 -maxdepth 1 -type d | \
    	sed 's|$(S_DIR)/||'); do \
			mkdir -p "$(O_DIR)/$$dir"; \
		done

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(O_DIR)/%.o: $(S_DIR)/%.c
	$(CC) -o $@ -c $< $(CCFLAGS)

ccflags:
	@echo $(CCFLAGS)

ldflags:
	@echo $(LDFLAGS)

clean-obj:
	@-rm -f $(OBJS) $(DEPS)
	@-rmdir $(O_DIR)

clean-bin:
	@-rm -f $(TARGET)
	@-rmdir $(B_DIR)

clean: clean-obj clean-bin

clean-all: clean

run:
	$(TARGET) $(ARGS)

hard: clean all

hard-run: clean all run

help:
	@echo "Type:"
	@echo "  'make all'......................... Build project"
	@echo "  'make run'................ Run binary (if exists)"
	@echo "  'make clean-obj'.............. Clean object files"
	@echo "  'make clean'....... Clean binary and object files"
	@echo "  'make hard'...................... Clean and build"
	@echo ""
	@echo " Binary will be placed in '$(TARGET)'"


.PHONY: clean clean-obj clean-all hard run hard-run ccflags ldflags help
