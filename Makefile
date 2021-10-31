
VERSION := 0.0.1
CC      :=  gcc
CFLAGS  := -Wall -O3 -g -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -ggdb -fno-omit-frame-pointer 
LDFLAGS := 

BUILDDIR := build
SOURCEDIR := src
HEADERDIR := src

NAME := qalam
BINARY := qalam

ECHO := echo
RM := rm -rf
MKDIR := mkdir
INSTALL := install

SOURCES := $(shell find $(SOURCEDIR) -name '*.c')

OBJECTS := $(addprefix $(BUILDDIR)/,$(SOURCES:$(SOURCEDIR)/%.c=%.o))

$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BINARY)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I $(HEADERDIR) -I $(dir $<) -c $< -o $@

all: $(BINARY)

clean: 
	$(RM) $(BINARY) $(OBJECTS)

setup: 
	$(MKDIR) -p $(BUILDDIR)

# Builder will call this to install the application before running.
install:
	echo "Installing ..."
	sudo debuild -b -uc -us
	sudo gdebi -n ../qalam_1.0.0-1_amd64.deb

# Builder uses this target to run your application.
run: $(BINARY)
	./qalam test1.q
