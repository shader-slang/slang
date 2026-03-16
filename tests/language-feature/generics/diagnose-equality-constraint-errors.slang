//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test error 30405: non-dependent equality constraint.
// Duplicate CHECK lines: the compiler emits each diagnostic twice
// (once during initial checking and once during re-checking).

interface IFoo
{
    associatedtype A;
}

// Valid: constraining an associated type to 'int' is allowed.
void foo<T : IFoo>(T val) where T.A == int
{
}

// Error 30405: both sides are non-dependent on the generic,
// so the equality constraint is meaningless.
void bar<T>(T val) where int == float
//CHECK:                     ^^ generic equality constraint requires at least one operand to be dependant on the generic declaration
//CHECK:                     ^^ generic equality constraint requires at least one operand to be dependant on the generic declaration
//CHECK:                 ^^^ type 'int' cannot be constrained by a type equality
//CHECK:                 ^^^ type 'int' cannot be constrained by a type equality
//CHECK:                        ^^^^^ type 'float' cannot be constrained by a type equality
//CHECK:                        ^^^^^ type 'float' cannot be constrained by a type equality
{
}
