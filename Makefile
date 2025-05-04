# Define the compiler
CC = gcc

# Define compile options
CFLAGS = -fsanitize=address -g -Wall

# Define target
TARGET = words

# Default rule
all: $(TARGET)

# Build rule
$(TARGET): words.c
	$(CC) $(CFLAGS) words.c -o $(TARGET)

# Clean rule to remove compiled files
clean:
	rm -f $(TARGET)
