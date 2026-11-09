// A call to an overloaded instance method named from a static function reports only the
// error for the invalid member reference. The call to the erroneous callee is itself
// erroneous, so neither the `try` checks nor a member access on its result report more.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum MyError
{
    Failed
}

struct R
{
    int x;
}

struct Test
{
    int m(int x) throws MyError { return x; }
    int m(float x) throws MyError { return 0; }

    int n(int x) { return x; }
    int n(float x) { return 0; }

    R r(int x) { R v; v.x = x; return v; }
    R r(float x) { R v; v.x = 0; return v; }

    static int viaName() throws MyError
    {
        return try m(1);
/*CHECK:
                   ^ E30100
                   ^ type 'Test' cannot be used to refer to non-static member 'm'
*/
    }

    static int viaThis() throws MyError
    {
        return try this.m(1);
/*CHECK:
                        ^ E30120
                        ^ static function cannot refer to non-static member `m` via `this`
*/
    }

    static int nonThrowingViaName() throws MyError
    {
        return try n(1);
/*CHECK:
                   ^ E30100
                   ^ type 'Test' cannot be used to refer to non-static member 'n'
*/
    }

    static int memberOfErroneousCall()
    {
        return r(1).y;
/*CHECK:
               ^ E30100
               ^ type 'Test' cannot be used to refer to non-static member 'r'
*/
    }
}
