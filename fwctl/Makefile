.RECIPEPREFIX = >
CC = gcc
CFLAGS = -Wall -Wextra -O2 $(shell pkg-config --cflags libnl-genl-3.0)
LIBS = $(shell pkg-config --libs libnl-genl-3.0)

SRC = main.c parse.c print.c fwnl.c
HDR = parse.h print.h fwnl.h fw_common_draft.h fw_cmd_draft.h

fwctl: $(SRC) $(HDR)
> $(CC) $(CFLAGS) -o $@ $(SRC) $(LIBS)

clean:
> rm -f fwctl

.PHONY: clean
