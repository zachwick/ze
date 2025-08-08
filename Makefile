INSTALL_LOC ?= $(HOME)/.local/bin
CFLAGS += -std=c11 -Wall -Wextra -pedantic -O2 -Iinclude `pkg-config --cflags guile-3.0`
LIBS = `pkg-config --libs guile-3.0`

SRC = \
  src/main.c \
  src/terminal.c \
  src/status.c \
  src/syntax.c \
  src/row.c \
  src/edit.c \
  src/fileio.c \
  src/search.c \
  src/buffer.c \
  src/render.c \
  src/input.c \
  src/plugins.c \
  src/hooks.c \
  src/util.c

OBJ = $(SRC:.c=.o)

all: ze

build: ze

ze: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: build
	valgrind --leak-check=full --show-leak-kinds=all ./ze ze.c

install: build
	rm -f $(INSTALL_LOC)/ze
	cp ze $(INSTALL_LOC)/ze
	# Install example config to ~/.ze
	mkdir -p $(HOME)/.ze
	# Only copy example config if one doesn't already exist
	if [ ! -f $(HOME)/.ze/zerc.scm ]; then cp zerc.scm $(HOME)/.ze/; fi
	# Install example plugins to ~/.ze/plugins
	mkdir -p $(HOME)/.ze/plugins
	if [ -d plugins ]; then cp -R plugins/* $(HOME)/.ze/plugins/; fi
	# Install example templates to ~/.ze/templates
	mkdir -p $(HOME)/.ze/templates
	if [ -d templates ]; then cp -R templates/* $(HOME)/.ze/templates/; fi

clean:
	rm -f ze $(OBJ)
