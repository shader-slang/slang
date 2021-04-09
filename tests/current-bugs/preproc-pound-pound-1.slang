//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// GCC: <source>:1:9: error: '##' cannot appear at either end of a macro expansion. 
// Clang: <source>:1:21: error: '##' cannot appear at start of macro expansion
// Slang outputs Hello ## There;
#define POUND_POUND ##

Hello POUND_POUND There;
