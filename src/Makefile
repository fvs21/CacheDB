CC = gcc
IDIR = ../include
CFLAGS = -I$(IDIR)
ODIR = obj

_DEPS = cache.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = main.o client.o cache.o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ)) 

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cache: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o