// class-inheritance-invalid.slang

// Test that a class cannot inherit from a struct (error 30814).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct MyStruct
{
    int value;
}

class BadClass : MyStruct
//CHECK:         ^^^^^^^^ class 'BadClass' cannot inherit from type 'MyStruct' that is neither a class nor an interface
{
    int extra;
}
