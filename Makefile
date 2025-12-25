# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses

# Target executable name
TARGET = game

# Find all .c files in current directory
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
        $(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Compile .c files to .o files
%.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
        rm -f $(OBJS) $(TARGET)

# Rebuild everything
rebuild: clean all

#Update from Github
update:
        wget -O game.c https://raw.githubusercontent.com/Skyw>
        make

# Phony targets (not actual files)
.PHONY: all clean rebuild