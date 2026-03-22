SRC = $(shell find . -type f -name "*.c")
OBJ = $(SRC:.c=.o)

CFLAGS = -g -std=c11 -Wpedantic -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE=1
LDFLAGS =

all: vis

vis: $(OBJ)
	cc $(LDFLAGS) -o $@ $^

.PHONY: clean format

clean:
	rm -rf *.o vis

format:
	clang-format --fail-on-incomplete-format -i $(shell find . -type f -name "*.c" -o -name "*.h")

run: all
	./vis
