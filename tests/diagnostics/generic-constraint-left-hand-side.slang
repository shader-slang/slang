//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Check that we can issue diagnostic on invalid left hand side of a type constraint.

void f<T>() where int : IInteger
//diag:           ^^^ type 'int' is not a valid left hand side of a type constraint
//diag:           ^^^ type 'int' is not a valid left hand side of a type constraint
{}
