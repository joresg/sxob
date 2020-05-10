# dmenu - dynamic menu
# See LICENSE file for copyright and license details.

include config.mk

SRC = sxob.c
OBJ = $(SRC:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $<

$(OBJ): config.mk

sfm: sxob.o
	$(CC) -o $@ sxob.o $(LDFLAGS)

clean:
	rm -f sxob $(OBJ)

.PHONY: all options clean
