IDIR = include
CC = gcc
CFLAGS = -I$(IDIR)
ODIR = obj

_DEPS = aes.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = aes.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: $(ODIR) spe.exe

$(ODIR):
	@if not exist $(ODIR) mkdir $(ODIR)

$(ODIR)/%.o: src/%.c $(DEPS)
	@echo "Compiling $< to $@"
	$(CC) -c -o $@ $< $(CFLAGS)

spe.exe: $(OBJ)
	@echo "Linking object files to create spe.exe"
	$(CC) -o spe.exe $(OBJ) $(CFLAGS) -lm

.PHONY: clean

clean:
	@if exist $(ODIR) del /Q $(ODIR)\*.o
	@if exist spe.exe del spe.exe