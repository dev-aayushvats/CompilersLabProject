# Compiler to use
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -std=c++11

# Target executable name
TARGET = compiler

# Source file
SOURCES = main.cpp

# Object file
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean