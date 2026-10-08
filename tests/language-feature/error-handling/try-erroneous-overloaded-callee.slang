// A `try` whose callee is an overloaded instance method named from a static function
// reports only the error for the invalid member reference. The call to the erroneous
// callee is itself erroneous, so the `try` checks do not run on it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum MyError
{
    Failed
}

struct Test
{
    int m(int x) throws MyError { return x; }
    int m(float x) throws MyError { return 0; }

    int n(int x) { return x; }
    int n(float x) { return 0; }

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
}
