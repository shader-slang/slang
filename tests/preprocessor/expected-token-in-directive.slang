//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Test 15102: expected token in preprocessor directive
// A failed filename parse must not attempt an include lookup with an empty path.

// #include without filename
#include
//diag: ^ preprocessor parse error
//diag: ^ expected 'string literal' in 'include' directive
