CC := gcc
MAKE := make
AR := ar

# Useful directories.
PROJECT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

DEPS_DIR = $(PROJECT_DIR)deps
FUSE_DIR = $(DEPS_DIR)/fuse
SRC_DIR := src
INCLUDE_DIR := $(PROJECT_DIR)include
BUILD_DIR := $(PROJECT_DIR)build
TESTS_DIR := $(PROJECT_DIR)tests

CFLAGS := -I$(FUSE_DIR)/include -I$(INCLUDE_DIR) -Wall -Werror -g \
		   -fdiagnostics-color=always 
LDFLAGS := -L$(FUSE_DIR)/lib -L$(BUILD_DIR) \
		   -fuse-ld=gold \
		   -Wl,-rpath=$(abspath $(FUSE_DIR)/lib)

LDLIBS := -lfuse3 -lufs -lpthread -ldl

# project names.
PROJ := ufs

ARCHIVE := $(BUILD_DIR)/libufs.a

# Place compilation targets here.
OBJECTS := $(BUILD_DIR)/src/ufs_image.o # $(BUILD_DIR)/src/ufs_index.o
GLOBAL_HEADERS := $(INCLUDE_DIR)/ufs_defs.h

# Entry point to each executable target.
MAIN_ENTRY := $(BUILD_DIR)/$(SRC_DIR)/main.o

all: $(PROJ) test

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
