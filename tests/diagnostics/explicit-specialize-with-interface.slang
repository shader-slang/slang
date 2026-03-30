// Explicitly specializing a generic function with an interface type
// (e.g. genericFunc<IFoo>(...)) should produce error 33180, not ICE.
// The implicit inference path (genericFunc(obj, ...)) correctly handles
// dynamic dispatch without specialization.
//
// See #10293.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

interface IFoo
{
    float calc(float x);
}

struct A : IFoo { float calc(float x) { return x * 2.0; } }
struct B : IFoo { float calc(float x) { return x + 10.0; } }

float genericFunc<T : IFoo>(T obj, float x)
{
    return obj.calc(x);
//CHECK:           ^ cannot specialize generic with existential type
//CHECK:           ^ specializing 'genericFunc' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
}

IFoo makeObj(int id)
{
    if (id == 0) return A();
    else return B();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFoo obj = makeObj(id);
    float result = genericFunc<IFoo>(obj, 3.0);
    outputBuffer[0] = result;
}
