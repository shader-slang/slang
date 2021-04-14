//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// If a macro can take a single parameter, it is valid to pass in 'nothing'.
// Slang outputs an error about the wrong amount of parameters
// Correct output: a b

#define A(x) a x b

A()

