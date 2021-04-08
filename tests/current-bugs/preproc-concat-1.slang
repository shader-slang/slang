//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

#define CONCAT(a, b) a ## b

// Correct output AB;
// Slang output
// ab;

#define A a
#define B b

CONCAT(A, B);
