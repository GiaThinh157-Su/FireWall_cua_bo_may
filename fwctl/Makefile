.RECIPEPREFIX = >
CC = gcc
CFLAGS = -Wall -Wextra -O2

SRC = main.c parse.c

fwctl: $(SRC) parse.h fw_common_draft.h
> $(CC) $(CFLAGS) -o $@ $(SRC)

clean:
> rm -f fwctl

.PHONY: clean
