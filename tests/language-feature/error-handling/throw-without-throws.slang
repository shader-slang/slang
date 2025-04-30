//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
enum MyError
{
    Fail,
    Success
}

extension MyError: IError
{
    static MyError createSuccessValue() { return Success; }
    bool isSuccessValue() { return this == Success; }
}

int g() throws MyError
{
    throw MyError.Fail;
}

void f()
{
    let n = try g();
    throw MyError.Fail;
}

// CHECK: error 30093
// CHECK: error 30113
