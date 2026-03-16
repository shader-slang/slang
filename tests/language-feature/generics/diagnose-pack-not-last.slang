//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Test: generic parameter declared after a variadic pack parameter
// produces error 30500.

// Type pack followed by a non-pack type parameter.
struct BadTypeAfterPack<each T, U> {}
//CHECK:                        ^ generic parameter 'U' cannot appear after a variadic pack parameter

// Value pack followed by a non-pack value parameter.
struct BadValueAfterPack<let each D : int, let N : int> {}
//CHECK:                                       ^ generic parameter 'N' cannot appear after a variadic pack parameter

// Type pack followed by a value parameter.
struct BadMixedAfterPack<each T, let N : int> {}
//CHECK:                             ^ generic parameter 'N' cannot appear after a variadic pack parameter
