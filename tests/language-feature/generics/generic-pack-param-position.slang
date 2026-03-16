//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Pack param followed by a non-pack param should error.
struct Bad1<each T, U> {}
//CHECK:            ^ generic parameter after a variadic pack parameter is not allowed
//CHECK:            ^ generic parameter 'U' cannot appear after a variadic pack parameter

// Value pack param followed by a non-pack param should error.
struct Bad2<let each D : int, let N : int> {}
//CHECK:                          ^ generic parameter after a variadic pack parameter is not allowed
//CHECK:                          ^ generic parameter 'N' cannot appear after a variadic pack parameter

// Type pack followed by a value parameter (mixed case).
struct Bad3<each T, let N : int> {}
//CHECK:                ^ generic parameter after a variadic pack parameter is not allowed
//CHECK:                ^ generic parameter 'N' cannot appear after a variadic pack parameter