//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that using a non-interface type as a generic constraint
// produces error 30401.

// Using a concrete type like 'int' as a constraint is invalid.
void foo<T : int>(T val)
//CHECK:     ^^^ type 'int' cannot be used as a constraint
//CHECK:     ^^^ type 'int' cannot be used as a constraint
{
}

// Using a struct as a constraint is also invalid.
struct MyStruct
{
    int x;
}

void bar<T : MyStruct>(T val)
//CHECK:     ^^^^^^^^ type 'MyStruct' cannot be used as a constraint
//CHECK:     ^^^^^^^^ type 'MyStruct' cannot be used as a constraint
{
}
