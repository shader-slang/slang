//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -E

// The final escaped newline reaches logical EOF without closing the header name.
//CHECK-NOT: include file
//CHECK: end of file in literal
//CHECK-NOT: include file
//CHECK: expected 'string literal' in 'include' directive
//CHECK-NOT: include file
#include<not-a-real-header.h\
