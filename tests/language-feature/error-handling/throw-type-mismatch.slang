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

extension MyError1: IError
{
    static MyError1 createSuccessValue() { return Success; }
    bool isSuccessValue() { return this == Success; }
}

extension MyError2: IError
{
    static MyError2 createSuccessValue() { return Success; }
    bool isSuccessValue() { return this == Success; }
}

int g() throws MyError1
{
    throw MyError2.Fail;
}

// CHECK: error 30114
