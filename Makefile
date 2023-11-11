VERSION := 0.0.1
CC      :=  gcc
CFLAGS  := -pedantic -Wall -Wextra -Wno-unused-parameter
LDFLAGS := -lm

BUILDDIR := build
SOURCEDIR := src
HEADERDIR := src

NAME := qalam
BINARY := qalam

RM := rm -rf
MKDIR := mkdir

ifeq ($(OS),Windows_NT)
	ifeq ($(ARCH), ARM64)
		CC = arm-none-eabi-gcc
	else
		CC = gcc
		CFLAGS += -DWINDOWS
		LDFLAGS += -lws2_32 -lShlwapi
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		AR=gcc-ar
		CFLAGS += -DLINUX -D_XOPEN_SOURCE=700 -D_GNU_SOURCE
		LDFLAGS += -Wl,-rpath=./
	endif
	ifeq ($(UNAME_S),Darwin)
		AR=ar
		CFLAGS += -DDARWIN
	endif
	CFLAGS += -fPIC
	LDFLAGS += -ldl
endif

DEBUG ?= 0
ifeq ($(DEBUG),0)
	CFLAGS += -O2 -DNDEBUG
else
	CFLAGS += -g -DDEBUG
endif

ifeq ($(USE_MALLOC)),1)
	CFLAGS += -DUSE_MALLOC
endif

SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
OBJECTS := $(addprefix $(BUILDDIR)/,$(SOURCES:$(SOURCEDIR)/%.c=%.o))

$(BINARY): $(OBJECTS)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
	@echo CC LINK $@

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -I $(HEADERDIR) -I $(dir $<) -c $< -o $@
	@echo CC $<

all: $(BINARY)

clean: 
	$(RM) $(BINARY) $(OBJECTS)

setup: 
	$(MKDIR) -p $(BUILDDIR)

# Builder uses this target to run your application.
test: $(BINARY)
	./qalam ./test/test.q
