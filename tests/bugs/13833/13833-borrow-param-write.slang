// A borrow parameter (`__constref`) is an immutable borrow of its argument: the body may not
// write it, because on targets that lower a borrow to a pointer the write would reach the
// caller's storage. Reads still work, and a legacy (C-style) `in` parameter stays a mutable
// local copy.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

struct S { int a[4]; }

[noinline] int bump(__constref S s) { s.a[0] = 7; return s.a[0]; }
//diag:                                      ^ left of '=' is not an l-value
//diag:                                      ^ left of '=' is not an l-value.
[noinline] int bumpInt(__constref int x) { x = 9; return x; }
//diag:                                      ^ left of '=' is not an l-value
//diag:                                      ^ left of '=' is not an l-value.
[noinline] int readBorrow(__constref S s) { return s.a[0]; }
[noinline] int bumpLegacyIn(in int x) { x = x + 10; return x; }
