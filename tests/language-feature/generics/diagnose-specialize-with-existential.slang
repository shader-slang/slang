// Test that the compiler emits an error when a generic function is called
// with arguments whose types cannot satisfy the generic constraints, because
// the call is ambiguous between overloads.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    float calc(float x);
}

interface IBar
{
    int getVal();
}

struct A : IFoo
{
    float calc(float x) { return x * x; }
}

T identity<T : IFoo>(T val) { return val; }
T identity<T : IBar>(T val) { return val; }

void test()
{
    A a;
    // A conforms to IFoo but not IBar, yet both overloads are considered.
    // Providing explicit wrong specialization triggers error.
    identity<int>(42);
/*CHECK:
    ^^^^^^^^ unimplemented feature in Slang compiler: no applicable generic
*/
}
