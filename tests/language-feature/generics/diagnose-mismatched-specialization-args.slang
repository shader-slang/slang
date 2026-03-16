// Test that the compiler emits an error when the wrong number of
// specialization arguments are provided to a generic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int getValue();
}

struct MyType : IFoo
{
    int getValue() { return 1; }
}

void genericFunc<T : IFoo>(T a)
{
}

void test()
{
    MyType m;
    genericFunc<MyType, MyType>(m);
/*CHECK:
    ^^^^^^^^^^^ too many arguments to call
*/
}
