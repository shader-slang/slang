//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

#define CONCAT(a, b) a ## b

#define A a
#define B b

#define A2 A
#define B2 B

// Gives error (as trys to concat unexpanded input)
// <source>:11:1: error: pasting formed ')CONCAT', an invalid preprocessing token
//
// Slang output: aabb ;

CONCAT(CONCAT(A2, A2), CONCAT(B2, B2));
