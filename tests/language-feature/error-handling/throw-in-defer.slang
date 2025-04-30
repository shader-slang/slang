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

void f() throws MyError
{
    defer {
        // Throw isn't allowed to escape defer for the same reason as 'return',
        // it'd prevent other defer statements from running. This is legal if
        // you catch it, though.
        throw MyError.Fail;
    }
}

// CHECK: error 30111
