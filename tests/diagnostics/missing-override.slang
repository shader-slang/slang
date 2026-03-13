//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IFoo
{
    int getVal() { return 0; }
}

struct Impl : IFoo
{
    // Missing override for getVal, which should trigger a diagnostic.
    int getVal() { return 1; }
//CHECK:^^^^^^ missing 'override' keyword for methods that overrides the default implementation in the interface
//CHECK:^^^^^^ missing 'override' keyword for methods that overrides the default implementation in the interface
}

struct Impl2 : IFoo
{
    // Overriding getVal with a different signature should also trigger a diagnostic.
    override int getVal(int x) { return x; }
//CHECK:         ^^^^^^ 'getVal' marked as 'override' is not overriding any base declarations
//CHECK:         ^^^^^^ 'getVal' marked as 'override' is not overriding any base declarations
}