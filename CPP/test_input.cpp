#include <iostream>

// Global variables
int globalVar = 5;

int main() {
    // Variable declarations
    int x = 10;
    int y;
    char c = 'a';
    
    // Assignment
    y = 5 + x * 2;
    
    // If statement
    if (x > 10) {
        y = 100;
    } else if (x == 10) {
        y = 50;
    } else {
        y = 0;
    }
    
    // While loop
    int i = 0;
    while (i < 5) {
        y = y + i;
        i = i + 1;
    }
    
    // For loop
    for (int j = 0; j < 3; j = j + 1) {
        x = x + j;
    }
    
    return 0;
} 