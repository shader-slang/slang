//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Witness-constraint inference when the subject type argument is an interface
// type. Generic arguments still come from the subject's facets, and an equality
// constraint still narrows an interface-typed default to the concrete type it
// names (gh-13461).

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IBar<X>
{
    X v();
}

interface IFoo<X> : IBar<X>
{
}

struct S : IFoo<int>
{
    int v() { return 7; }
}

struct S5 : IFoo<int>
{
    int v() { return 5; }
}

U getFromFoo<T, U>(T t)
    where T : IFoo<U>
{
    return t.v();
}

U getFromBar<T, U>(T t)
    where T : IBar<U>
{
    return t.v();
}

interface IGet
{
    int get();
}

struct G : IGet
{
    int get() { return 3; }
}

int getDefaulted<T = IGet>(int x)
    where T == G
{
    return T().get() + x;
}

int sameAsDefault<T, U = IFoo<int>>(T t)
    where U == T
{
    return 1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo<int> a = S();
    IFoo<int> b = S5();
    S s;

    // CHECK: 7
    outputBuffer[0] = getFromFoo(a);
    // CHECK-NEXT: 5
    outputBuffer[1] = getFromBar(b);
    // CHECK-NEXT: 4
    outputBuffer[2] = getDefaulted(1);
    // CHECK-NEXT: 1
    outputBuffer[3] = sameAsDefault(s);
}
