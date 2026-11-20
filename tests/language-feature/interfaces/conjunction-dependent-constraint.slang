// conjunction-dependent-constraint.slang

// Test a constraint that depends on an earlier conjunction: `T : IFoo<R>` needs the witnesses
// of both constraints flattened from `R : IA & IB`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

interface IA
{
    int getA();
}

interface IB
{
    int getB();
}

struct Value : IA & IB
{
    int v;
    int getA() { return v; }
    int getB() { return v * 10; }
}

interface IFoo<R : IA & IB>
{
    int apply(R r);
}

struct Foo<R : IA & IB> : IFoo<R>
{
    int offset;
    int apply(R r) { return r.getA() + r.getB() + offset; }
}

// The dependent constraint follows the conjunction in the same generic parameter list.
int applyFoo<R : IA & IB, T : IFoo<R>>(T foo, R r)
{
    return foo.apply(r);
}

// The same, with the conjunction written as separate `where` clauses.
int applyFooWhere<R, T>(T foo, R r)
    where R : IA
    where R : IB
    where T : IFoo<R>
{
    return foo.apply(r);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: 11
// CHECK: 12
// CHECK: 22
// CHECK: 23
// CHECK: 33
// CHECK: 34
// CHECK: 44
// CHECK: 45

[numthreads(4, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    Value r = { tid.x + 1 };
    Foo<Value> foo = { 0 };
    Foo<Value> foo1 = { 1 };
    outputBuffer[tid.x * 2 + 0] = applyFoo<Value, Foo<Value>>(foo, r);
    outputBuffer[tid.x * 2 + 1] = applyFooWhere<Value, Foo<Value>>(foo1, r);
}
