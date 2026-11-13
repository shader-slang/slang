//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -E

// An unterminated physical header name is invalid input, so the preprocessor
// must not remove its final path character and attempt to open the truncated name.
#include<not-a-real-header.h

//CHECK-NOT: include file
//CHECK: newline in literal
//CHECK: expected 'string literal' in 'include' directive
//CHECK-NOT: include file
