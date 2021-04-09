//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

#define CONCAT(a, b) a ## b

#define A a
#define B b

#define A2 A
#define B2 B

// Correct output: a A2B2 b;
// Slang output 
// a ab b ;

CONCAT(A2 A2, B2 B2);
