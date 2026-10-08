//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A conversion to the type a throwing call already returns is still a call, so a `try` on the
// conversion does not cover the throwing call inside it (#13489). The throwing call carries its
// own `try` instead.

enum MyError
{
    Failed
}

int f() throws MyError { throw MyError.Failed; }

int constructorStyleIdentity() throws MyError
{
    return try int(f());
//CHECK:            ^ callee may throw, use 'try'
//CHECK:            ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int castStyleIdentity() throws MyError
{
    return try (int)f();
//CHECK:             ^ callee may throw, use 'try'
//CHECK:             ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int constructorStyleIdentityExplicit() throws MyError
{
    return int(try f());
}

int castStyleIdentityExplicit() throws MyError
{
    return (int)(try f());
}
