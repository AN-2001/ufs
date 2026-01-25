CC = gcc

# Useful directories.
PROJECT_DIR ?= $(abspath ..)/

DEPS_DIR = $(PROJECT_DIR)deps
INCLUDE_DIR := $(PROJECT_DIR)include
SRC_DIR := $(PROJECT_DIR)src
BUILD_DIR := $(PROJECT_DIR)build
TEST_DIR := $(PROJECT_DIR)tests

FUSE_DIR = $(DEPS_DIR)/fuse
CMOCKA_DIR = $(DEPS_DIR)/cmocka

CFLAGS := -I$(CMOCKA_DIR)/include -I$(FUSE_DIR)/include \
		  -I$(INCLUDE_DIR) \
		  -I$(SRC_DIR) \
		  -Wall -Werror -g -fdiagnostics-color=always

LDFLAGS :=  -L$(CMOCKA_DIR)/lib -L$(FUSE_DIR)/lib -L$(BUILD_DIR) \
			-Wl,-rpath=$(abspath $(CMOCKA_DIR)/lib) \
			-Wl,-rpath=$(abspath $(FUSE_DIR)/lib)

LDLIBS := -lcmocka -lfuse3 -lufs -lpthread -ldl

# project names.
TESTS := test_ufs_core

# Place compilation targets here.
SOURCES = $(wildcard *.c)
OBJECTS := $(BUILD_DIR)/tests/utils.o

all: $(TESTS) depend $(BUILD_DIR)/ufs_tests.sh 

depend: .depend

.depend: $(SOURCES)
	$(CC) $(CFLAGS) -MM $^ > "$@"

include .depend

test_ufs_core: $(BUILD_DIR)/tests/test_ufs_core.o $(OBJECTS) 
	@mkdir -p $(BUILD_DIR)/tests
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $(BUILD_DIR)/tests/$@

$(BUILD_DIR)/tests/%.o: %.c 
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/ufs_tests.sh: ./ufs_tests.sh
	cp $< $@

.PHONY: all clean

clean:
	rm -rf $(BUILD_DIR)
