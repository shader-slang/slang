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

void f() throws MyError
{
    defer {
        let n = try g();
    }
}

// CHECK: error 30112
