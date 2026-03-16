// Test that the compiler emits an error when a generic function is
// explicitly specialized with a type that does not conform to any
// of the overloaded constraints, resulting in no applicable generic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

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
    // 'int' conforms to neither IFoo nor IBar, so no overload applies.
    identity<int>(42);
/*CHECK:
    ^^^^^^^^ unimplemented feature in Slang compiler: no applicable generic
*/
}
