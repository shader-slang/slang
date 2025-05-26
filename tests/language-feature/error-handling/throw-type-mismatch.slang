//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
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
}

// CHECK: error 30116
