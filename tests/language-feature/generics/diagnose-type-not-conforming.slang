// Test that the compiler emits an error when a type argument does not
// conform to the required interface constraint on a generic parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int getValue();
}

struct Bar
{
    int x;
}

void useGeneric<T : IFoo>(T val)
{
}

void test()
{
    Bar b;
    useGeneric<Bar>(b);
/*CHECK:
    ^^^^^^^^^^ type argument doesn't conform to interface
*/
}
