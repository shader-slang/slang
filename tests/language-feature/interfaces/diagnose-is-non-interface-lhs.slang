// Test E30304: 'is'/'as' on unrelated concrete types.

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
//CHECK:       ^^ 'is'/'as' on unrelated concrete types
//CHECK:       ^^ 'is'/'as' on unrelated concrete types will never succeed.
    let c = o as Impl;
//CHECK:      ^^ 'is'/'as' on unrelated concrete types
//CHECK:      ^^ 'is'/'as' on unrelated concrete types will never succeed.
}
