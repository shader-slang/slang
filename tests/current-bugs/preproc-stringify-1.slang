//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

#define A a
#define B b

// Correct output 
// "A B"
// Slang output 
// # a b ;

#define STRINGIFY(x) #x
STRINGIFY(A B);