//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
enum MyError1
{
    Fail,
    Success
}

enum MyError2
{
    Fail,
    Success
}

int g() throws MyError1
{
    throw MyError2.Fail;
}

// CHECK: error 30116
