// Program to test various features of the compiler
int a = 10;         // Simple integer declaration with initialization
int b;              // Integer declaration without initialization
char c = 'x';       // Character declaration with initialization
char d;             // Character declaration without initialization

/* Multi-line comment explaining the next section
   This tests arithmetic operations and assignments */
b = a + 3 * 2 - 8 / 4;  // Complex arithmetic expression with precedence
int e = b + 5;          // Using a previously assigned variable

// Assigning a char to another char variable
d = c;                  // Valid char-to-char assignment

int f = e * 2 + a - b;  // Another arithmetic operation using multiple variables