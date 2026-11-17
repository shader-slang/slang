// A borrow parameter (`__constref`) is an immutable borrow of its argument: the body may not
// write it, because on targets that lower a borrow to a pointer the write would reach the
// caller's storage. Reads still work, and a legacy (C-style) `in` parameter stays a mutable
// local copy.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-experimental-feature

struct S { int a[4]; }

[noinline] int bump(__constref S s) { s.a[0] = 7; return s.a[0]; }
//diag:                                      ^ left of '=' is not an l-value
//diag:                                      ^ left of '=' is not an l-value.
[noinline] int bumpInt(__constref int x) { x = 9; return x; }
//diag:                                      ^ left of '=' is not an l-value
//diag:                                      ^ left of '=' is not an l-value.
[noinline] int readBorrow(__constref S s) { return s.a[0]; }
[noinline] int bumpLegacyIn(in int x) { x = x + 10; return x; }

// A borrow is a read-only value rather than a memory location, so it cannot be forwarded to a
// `__ref_readonly` parameter.
int leaf(__ref_readonly int x) { return x; }
[noinline] int forwardBorrow(__constref S s) { return leaf(s.a[0]); }
//diag:                                                       ^ argument must be l-value
//diag:                                                       ^ argument passed to parameter '0' must be l-value.
