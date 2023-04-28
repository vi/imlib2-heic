include commands.mk

OPTS    := -O3
CFLAGS  := -std=c99 $(OPTS) $(shell pkg-config libheif --cflags) -fPIC -Wall
LDFLAGS := $(shell pkg-config --libs libheif imlib2)


SRC = $(wildcard *.c)
OBJ = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP = $(SRC:.c=.d)

LIBDIR    ?= $(shell pkg-config --variable=libdir imlib2)
LOADERDIR ?= $(LIBDIR)/imlib2/loaders/

ifndef DISABLE_DEBUG
	CFLAGS += -ggdb
endif

.PHONY: all clean

all: heic.so

heic.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS) 
	cp $@ $@.debug
	strip $@

%.o: %.c
	$(CC) -Wp,-MMD,$*.d -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(DEP)
	$(RM) $(OBJ)
	$(RM) heic.so

install:
	$(INSTALL_DIR) $(DESTDIR)$(LOADERDIR)
	$(INSTALL_LIB) heic.so $(DESTDIR)$(LOADERDIR)

uninstall:
	$(RM) $(PLUGINDIR)/heic.so

-include $(DEP)

