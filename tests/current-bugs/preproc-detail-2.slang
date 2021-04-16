//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// Macro parameters must have unique names

#define A(x, x) a x b x c

A(,)

