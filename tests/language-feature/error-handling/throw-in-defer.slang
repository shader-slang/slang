//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
enum MyError
{
    Fail
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

// CHECK: error 30113
