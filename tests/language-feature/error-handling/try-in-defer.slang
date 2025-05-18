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

void f() throws MyError
{
    defer {
        let n = try g();
    }
}

// CHECK: error 30114
