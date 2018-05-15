ze: ze.c
	$(CC) ze.c -o ze -Wall -Wextra -pedantic -std=c99
test: ze
	./ze ze.c
