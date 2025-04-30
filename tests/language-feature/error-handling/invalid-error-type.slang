//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
enum MyError
{
    Fail,
    Success
}

int g() throws MyError
{
    throw MyError.Fail;
}

// CHECK: error 30115
