// Test `dyn` qualifier on variable/parameter declarations.
//
// The `some` qualifier is not yet implemented (SP#024, tracked by #7115).
// Only `dyn` on var/param declarations is currently supported.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

dyn interface IFoo
{
    int get();
}

struct A : IFoo
{
    int val;
    int get() { return val; }
}

struct B : IFoo
{
    int val;
    int get() { return val * 2; }
}

struct Container
{
    dyn IFoo field;
}

int callFoo(dyn IFoo x)
{
    return x.get();
}

void makeFoo(out dyn IFoo result)
{
    A a = {10};
    result = a;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    A a = {3};
    dyn IFoo x = a;
    outputBuffer[0] = x.get();
    // CHECK: 3

    B b = {5};
    dyn IFoo y = a;
    y = b;
    outputBuffer[1] = y.get();
    // CHECK: 10

    outputBuffer[2] = callFoo(a);
    // CHECK: 3

    Container c;
    c.field = a;
    outputBuffer[3] = c.field.get();
    // CHECK: 3

    dyn IFoo z;
    makeFoo(z);
    outputBuffer[4] = z.get();
    // CHECK: 10
}
