//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

typealias Alias = int;

// Resolving the alias must not make a fixed type look dependent on the generic declaration.
void invalid<T>() where Alias == int {}
//CHECK: generic equality constraint requires at least one operand to be dependant on the generic declaration
//CHECK: type 'Alias' cannot be constrained by a type equality
//CHECK: type 'int' cannot be constrained by a type equality
