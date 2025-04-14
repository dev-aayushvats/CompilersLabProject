# Compiler to use
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -std=c++17

# Target executable name
TARGET = compiler

# Source file
SOURCES = main.cpp

# Object file
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = lexer.h parser.h semantic.h

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the compiler against the test file
test: $(TARGET)
	./$(TARGET) test_input.cpp

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean test