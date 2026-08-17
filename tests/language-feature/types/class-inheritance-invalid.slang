// class-inheritance-invalid.slang

// Test that a class cannot inherit from a struct (error 30814).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct MyStruct
{
    int value;
}

class BadClass : MyStruct
//CHECK:         ^^^^^^^^ error E30814
{
    int extra;
}
