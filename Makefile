CC = cc
CFLAGS := -g -Wall -Wextra -I deps -I src
TEST_SRC = $(shell find src tests deps -name '*.c')


compile:
	$(CC) $(CFLAGS) $(TEST_SRC) -std=gnu99 -o test.o

test:
	make compile
	./test.o

valgrind:
	make compile
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./test.o
