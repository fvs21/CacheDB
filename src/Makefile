CC = gcc
IDIR = ../include
CFLAGS = -I$(IDIR) -g
ODIR = ../build/obj
BUILD_DIR = ../build

_DEPS = cache.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = main.o cache.o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ)) 

$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p $(ODIR)
	@mkdir -p $(BUILD_DIR)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: $(BUILD_DIR)/cache $(BUILD_DIR)/client

$(BUILD_DIR)/cache: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BUILD_DIR)/client: $(ODIR)/client.o
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean

debug:
	$(CC) -g $(CFLAGS) -I$(IDIR) main.c cache.c -o debCache

clean:
	rm -f $(ODIR)/*.o