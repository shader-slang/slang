//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test error 30404: invalid type in equality constraint.
// Test error 30405: non-dependent equality constraint.

interface IFoo
{
    associatedtype A;
}

// Error 30404: 'int' is not a proper type for the right-hand side
// of a generic equality constraint (it should be an associated type or
// type parameter).
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
