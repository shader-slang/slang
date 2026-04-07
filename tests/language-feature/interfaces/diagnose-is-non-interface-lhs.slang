// Test E30300: 'is'/'as' operator on a concrete (non-interface) typed expression.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[anyValueSize(8)]
interface IFoo
{
    int get();
}

struct Impl : IFoo
{
    int val;
    int get() { return val; }
}

struct Other
{
    int x;
}

void test()
{
    Other o;
    bool b = o is Impl;
//CHECK:       ^^ 'is'/'as' on unrelated concrete types is always false
//CHECK:       ^^ 'is'/'as' comparison between unrelated concrete types is always false.
    let c = o as Impl;
//CHECK:      ^^ 'is'/'as' on unrelated concrete types is always false
//CHECK:      ^^ 'is'/'as' comparison between unrelated concrete types is always false.
}
