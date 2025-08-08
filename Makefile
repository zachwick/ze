INSTALL_LOC ?= $(HOME)/.local/bin
CFLAGS = `pkg-config --cflags guile-3.0`
LIBS = `pkg-config --libs guile-3.0`

all: ze

build: ze

ze: ze.o
	$(CC) $< -o $@ -Wall $(LIBS)

ze.o: ze.c
	$(CC) -std=c11 -c $< -o $@  -Wall -Wextra -pedantic $(CFLAGS)

test: build
	valgrind --leak-check=full --show-leak-kinds=all ./ze ze.c

install: build
	rm -f $(INSTALL_LOC)/ze
	cp ze $(INSTALL_LOC)/ze
	# Install example plugins to ~/.ze/plugins
	mkdir -p $(HOME)/.ze/plugins
	if [ -d plugins ]; then cp -R plugins/* $(HOME)/.ze/plugins/; fi

clean:
	rm ze
	rm ze.o
