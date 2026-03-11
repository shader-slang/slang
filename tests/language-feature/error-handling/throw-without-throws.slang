//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
enum MyError
{
    Fail
}

int g() throws MyError
{
    throw MyError.Fail;
}

void f()
{
    let n = try g();
//CHECK:    ^^^ uncaught 'try' in non-throwing function
//CHECK:    ^^^ the current function or environment is not declared to throw any errors, but the 'try' clause is not caught
    throw MyError.Fail;
/*CHECK:
    ^^^^^ uncaught 'throw' in non-throwing function
    ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
*/
}
