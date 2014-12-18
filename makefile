include commands.mk

BPGDIR=/mnt/src/p/libbpg-0.9.3

OPTS    := -O2
CFLAGS  := -std=c99 $(OPTS) $(shell imlib2-config --cflags) -fPIC -Wall -I${BPGDIR}
LDFLAGS := $(shell imlib2-config --libs) -L${BPGDIR} -lbpg


SRC = $(wildcard *.c)
OBJ = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP = $(SRC:.c=.d)

LIBDIR    ?= $(shell pkg-config --variable=libdir imlib2)
LOADERDIR ?= $(LIBDIR)/imlib2/loaders/

ifndef DISABLE_DEBUG
	CFLAGS += -ggdb
endif

.PHONY: all clean

all: bpg.so

bpg.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -Wp,-MMD,$*.d -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(DEP)
	$(RM) $(OBJ)
	$(RM) bpg.so

install:
	$(INSTALL_DIR) $(DESTDIR)$(LOADERDIR)
	$(INSTALL_LIB) bpg.so $(DESTDIR)$(LOADERDIR)

uninstall:
	$(RM) $(PLUGINDIR)/bpg.so

-include $(DEP)

