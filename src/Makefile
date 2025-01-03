CC = gcc
IDIR = ../include
CFLAGS = -I$(IDIR) -g
ODIR = obj

_DEPS = cache.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = main.o cache.o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ)) 

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: cache client

cache: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

client: obj/client.o
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean

debug:
	$(CC) -g $(CFLAGS) -I$(IDIR) main.c cache.c -o debCache

clean:
	rm -f $(ODIR)/*.o