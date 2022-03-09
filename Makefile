PROJECT_DIR = $(CURDIR)
BIN_DIR = $(PROJECT_DIR)/bin
SRC_DIR = $(PROJECT_DIR)/src
NO_WARN_UNUSED = #-Wno-unused-but-set-variable -Wno-unused-result -Wno-unused-variable

export

all: main tests

main: 
	cd $(SRC_DIR)/main && $(MAKE) SLGEWOS

	
tests: O0 O1 O2 O3


O0: 
	cd $(SRC_DIR)/tests && $(MAKE) SLGEWOS_O0

O1: 
	cd $(SRC_DIR)/tests && $(MAKE) SLGEWOS_O1

O2: 
	cd $(SRC_DIR)/tests && $(MAKE) SLGEWOS_O2

O3: 
	cd $(SRC_DIR)/tests && $(MAKE) SLGEWOS_O3

# Variante sequenziale
O3_seq: 
	cd $(SRC_DIR)/tests && $(MAKE) SLGEWOS_O3_seq
