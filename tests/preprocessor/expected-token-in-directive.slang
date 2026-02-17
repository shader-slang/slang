//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):

// Test 15102: expected token in preprocessor directive
// This tests the ExpectRaw helper function with TokenInDirective

// #include without filename
#include
//diag: ^ preprocessor parse error
//diag: ^ expected 'string literal' in 'include' directive
