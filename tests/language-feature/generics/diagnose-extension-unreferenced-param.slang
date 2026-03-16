// Test that the compiler emits an error when a generic parameter in an
// extension is not referenced by the extension's target type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int getValue();
}

struct MyStruct
{
    int x;
}

// generic parameter T is not referenced by extension target type.
extension<T> MyStruct
//CHECK:  ^ generic parameter 'T' is not referenced by extension target type 'MyStruct'
{
    int unusedGenericMethod() { return 0; }
}
