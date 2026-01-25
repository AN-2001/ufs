CC := gcc
MAKE := make
AR := ar

# Useful directories.
PROJECT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

DEPS_DIR := $(PROJECT_DIR)deps
FUSE_DIR := $(DEPS_DIR)/fuse
SQLITE_DIR := $(DEPS_DIR)/sqlite

SRC_DIR := src
INCLUDE_DIR := $(PROJECT_DIR)include
BUILD_DIR := $(PROJECT_DIR)build
TESTS_DIR := $(PROJECT_DIR)tests

CFLAGS := -I$(FUSE_DIR)/include -I$(SQLITE_DIR) -I$(INCLUDE_DIR) -Wall -Werror -g \
		   -fdiagnostics-color=always 
LDFLAGS := -L$(FUSE_DIR)/lib -L$(BUILD_DIR) \
		   -Wl,-rpath=$(abspath $(FUSE_DIR)/lib)

LDLIBS := -lfuse3 -lufs -lpthread -ldl

# Project names.
PROJ := ufs

ARCHIVE := $(BUILD_DIR)/libufs.a

# Place compilation targets here.

SOURCES := $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c)) $(SQLITE_DIR)/sqlite3.c

OBJECTS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SOURCES)) 

GLOBAL_HEADERS := $(INCLUDE_DIR)/ufs_core.h

# Entry point to each executable target.
MAIN_ENTRY := $(BUILD_DIR)/$(SRC_DIR)/main.o

all: $(PROJ) depend test

depend: .depend

.depend: $(SOURCES)
	$(CC) $(CFLAGS) -MM $^ > "$@"

include .depend

$(PROJ): $(ARCHIVE) $(MAIN_ENTRY)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(MAIN_ENTRY) $(LDFLAGS) $(LDLIBS) -o $(BUILD_DIR)/$@

test: $(ARCHIVE)
	$(MAKE) -C $(TESTS_DIR) PROJECT_DIR=$(PROJECT_DIR)

$(ARCHIVE): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: %.c $(wildcard %.h) $(GLOBAL_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: all clean test

clean:
	rm -rf $(BUILD_DIR)
