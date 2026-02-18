//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
enum MyError1
{
    Fail
}

enum MyError2
{
    Fail
}

int g() throws MyError1
{
    throw MyError2.Fail;
//CHECK:           ^^^^ throw type incompatible with error type
//CHECK:           ^^^^ the type `MyError2` of `throw` is not compatible with function's error type `MyError1`.
}
