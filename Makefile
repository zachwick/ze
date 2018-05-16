INSTALL_LOC = /usr/local/bin

build: ze

ze: ze.c
	$(CC) ze.c -o ze -Wall -Wextra -pedantic -std=c99

test: build
	valgrind --leak-check=full --show-leak-kinds=all ./ze ze.c

install:
	rm -f $(INSTALL_LOC)/ze
	cp pwd $(INSTALL_LOC)/ze

clean:
	rm ze
