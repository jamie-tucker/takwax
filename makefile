SRC_DIR = src
IDIR =$(SRC_DIR)/inc
CC=cc
CFLAGS=-I$(IDIR)

ODIR=obj

_OBJ = main.out 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.out: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

main.app: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.out *~ core $(IDIR)/*~ 