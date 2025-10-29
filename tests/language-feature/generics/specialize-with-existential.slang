//TEST:SIMPLE(filecheck=CHECK):

// Check that we can diagnose an error when the user attempts to specialize
// a generic with an existential type.

#language slang 2025

interface IFoo {}

struct Foo : IFoo{}

int genericFunc<T:IFoo>(T v)
{
    return 0;
}

void caller(IFoo f)
{
    // CHECK: ([[@LINE+1]]): error 33180: specializing 'genericFunc' with an existential type is not allowed
    genericFunc(f);
}