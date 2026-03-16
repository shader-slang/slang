//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that a generic value parameter without an explicit type annotation
// produces error 30623.
// Duplicate CHECK lines: the compiler emits each diagnostic twice
// (once during initial checking and once during re-checking).

struct Foo<let N>
//CHECK:       ^ a generic value parameter must be given an explicit type
//CHECK:       ^ a generic value parameter must be given an explicit type
//CHECK:       ^ generic value parameter type 'error' is not supported; only integer and enum types are allowed
//CHECK:       ^ generic value parameter type 'error' is not supported; only integer and enum types are allowed
{
}
