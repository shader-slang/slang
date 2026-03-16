//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that using an unsupported type for a generic value parameter
// produces error 30624.
// Duplicate CHECK lines: the compiler emits each diagnostic twice
// (once during initial checking and once during re-checking).

// Float is not a valid generic value parameter type.
struct Foo<let N : float>
//CHECK:       ^ generic value parameter type 'float' is not supported; only integer and enum types are allowed
//CHECK:       ^ generic value parameter type 'float' is not supported; only integer and enum types are allowed
{
}

// Struct is not a valid generic value parameter type.
struct Bar
{
    int x;
}

struct Baz<let N : Bar>
//CHECK:       ^ generic value parameter type 'Bar' is not supported; only integer and enum types are allowed
//CHECK:       ^ generic value parameter type 'Bar' is not supported; only integer and enum types are allowed
{
}
