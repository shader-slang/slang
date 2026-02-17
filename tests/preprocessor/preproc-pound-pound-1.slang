//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,diag=diag):-E

// GCC: <source>:1:9: error: '##' cannot appear at either end of a macro expansion.
// Clang: <source>:1:21: error: '##' cannot appear at start of macro expansion
// Old Slang outputs Hello ## There;
#define POUND_POUND ##
//diag:             ^^ invalid '##' position
//diag:             ^^ '##' is not allowed at the start of a macro body

Hello POUND_POUND There;

// CHECK: Hello ## There
