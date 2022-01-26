SRC_DIR = src
IDIR =$(SRC_DIR)/inc
CC=cc
CFLAGS=-I$(IDIR)

ODIR=$(SRC_DIR)/obj
# LDIR =$(SRC_DIR)/lib

# LIBS=-lm

# _DEPS = hellomake.h
# DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.out 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.out: $(SRC_DIR)/%.c #$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main.app: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.out *~ core $(IDIR)/*~ 