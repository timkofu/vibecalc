# Compiler
CC = gcc

# Project Name
PROJECT_NAME = vibecalc

# Source files
SRC = src/main.c \
      src/calculator.c

# Object files directory
OBJ_DIR = obj
OBJ = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Compiler flags
# -I: Include paths for raylib
# -Wall: Enable all common warnings
# -g: Include debug information
CFLAGS = -Wall -g -I/usr/local/include

# Linker flags
# -L: Library paths for raylib
# -l: Libraries to link (raylib and its system dependencies)
LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt

# Default target
all: $(OBJ_DIR) $(PROJECT_NAME)

$(PROJECT_NAME): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean target
clean:
	rm -f $(PROJECT_NAME) $(OBJ_DIR)/*.o
	rmdir $(OBJ_DIR) 2>/dev/null || true # Remove obj directory if empty

.PHONY: all clean