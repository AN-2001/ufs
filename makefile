CC = gcc

DEPS_DIR = deps
FUSE_DIR = $(DEPS_DIR)/fuse
SRC_DIR := src
INCLUDE_DIR := include

flags := -I$(FUSE_DIR)/include -I$(INCLUDE_DIR)
libs := -L$(FUSE_DIR)/lib -l:libfuse3.a -lpthread -ldl
PROJ := ufs
BUILD_DIR := bin

# Place compilation targets here.
RAW_OBJECTS := $(SRC_DIR)/main.o
OBJECTS := $(addprefix $(BUILD_DIR)/, $(RAW_OBJECTS))

$(PROJ): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(flags) $^ $(libs) -o $(BUILD_DIR)/$(PROJ)

$(BUILD_DIR)/%.o: %.c $(wildcard %.h)
	@mkdir -p $(dir $@)
	$(CC) -c $(flags) $< -o $@

clean:
	rm -rf $(BUILD_DIR)
