INSTALL_LOC = /usr/local/bin
CFLAGS = `pkg-config --cflags guile-3.0`
LIBS = `pkg-config --libs guile-3.0`

all: ze

build: ze

ze: ze.o
	$(CC) $< -o $@ -Wall $(LIBS)

ze.o: ze.c
	$(CC) -std=c99 -c $< -o $@  -Wall -Wextra -pedantic $(CFLAGS)

test: build
	valgrind --leak-check=full --show-leak-kinds=all ./ze ze.c

install: build
	rm -f $(INSTALL_LOC)/ze
	cp ze $(INSTALL_LOC)/ze

clean:
	rm ze
