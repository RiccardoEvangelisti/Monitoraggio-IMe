CC = gcc
DEBUG = -g 
OPTIMIZATION = -O3
CFLAGS = -Wall $(NO_WARN_UNUSED) $(DEBUG) $(OPTIMIZATION)
LIBS = -llapack -lblas -lm -lgfortran

all: SLGEWOS

SLGEWOS: SLGEWOS_main.c
	$(CC) $(CFLAGS) SLGEWOS_main.c -o $(BIN_DIR)/main/SLGEWOS_main $(LIBS)
