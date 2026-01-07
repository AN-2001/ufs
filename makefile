CC = gcc

DEPS_DIR = ./deps
FUSE_DIR = $(DEPS_DIR)/fuse
SRC_DIR = ./src
INCLUDE_DIR = ./include

flags = -I$(FUSE_DIR)/include -I$(INCLUDE_DIR)
libs = -L$(FUSE_DIR)/lib -l:libfuse3.a -lpthread -ldl
PROJ = unionfs

# Place compilation targets here.
OBJECTS = $(SRC_DIR)/main.o

$(PROJ): $(OBJECTS)
	$(CC) $(flags) $^ $(libs) -o $(PROJ)

%.o: %.c %.h
	$(CC) -c $(flags) $< -o $@

%.o: %.c
	$(CC) -c $(flags) $< -o $@

clean:
	rm -f *.o $(PROJ)
