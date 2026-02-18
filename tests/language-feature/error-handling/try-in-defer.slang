//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
enum MyError
{
    Fail
}

int g() throws MyError
{
    throw MyError.Fail;
}

void f() throws MyError
{
    defer {
        let n = try g();
//CHECK:        ^^^ 'try' requires 'catch' inside defer
//CHECK:        ^^^ 'try' expressions require a matching 'catch' inside a defer statement.
    }
}
