CC = gcc
DEBUG = -g 
CFLAGS = $(DEBUG) -Wall -Wextra
OPTIMIZATION_0 = -O0
OPTIMIZATION_1 = -O1
OPTIMIZATION_2 = -O2
OPTIMIZATION_3 = -O3

INCLUDE = -I$(PAPI_DIR)/include -I$(PAPI_DIR)/share/papi/testlib 
LIBS = -L$(PAPI_DIR)/share/papi/testlib -ltestlib $(PAPI_DIR)/lib/libpapi.a
# per qualche motivo scrivere: -L$PAPI_DIR/lib -lpapi non fa funzionare i contatori appio (risultano tutti 0)


all: SLGEWOS_O0 SLGEWOS_O1 SLGEWOS_O2 SLGEWOS_O3

SLGEWOS_O0: SLGEWOS_tests.c
	$(CC) $(CFLAGS) $(OPTIMIZATION_0) SLGEWOS_tests.c -o $(BIN_DIR)/tests/O0/SLGEWOS_tests_O0 $(INCLUDE) $(LIBS)
	
SLGEWOS_O1: SLGEWOS_tests.c
	$(CC) $(CFLAGS) $(OPTIMIZATION_1) SLGEWOS_tests.c -o $(BIN_DIR)/tests/O1/SLGEWOS_tests_O1 $(INCLUDE) $(LIBS)
	
SLGEWOS_O2: SLGEWOS_tests.c
	$(CC) $(CFLAGS) $(OPTIMIZATION_2) SLGEWOS_tests.c -o $(BIN_DIR)/tests/O2/SLGEWOS_tests_O2 $(INCLUDE) $(LIBS)
	
SLGEWOS_O3: SLGEWOS_tests.c
	$(CC) $(CFLAGS) $(OPTIMIZATION_3) SLGEWOS_tests.c -o $(BIN_DIR)/tests/O3/SLGEWOS_tests_O3 $(INCLUDE) $(LIBS)

# Variante sequenziale
SLGEWOS_O3_seq: SLGEWOS_tests_seq.c
	$(CC) $(CFLAGS) $(OPTIMIZATION_3) SLGEWOS_tests_seq.c -o $(BIN_DIR)/tests/O3/SLGEWOS_tests_O3_seq $(INCLUDE) $(LIBS)
