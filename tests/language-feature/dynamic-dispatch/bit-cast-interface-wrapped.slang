// `bit_cast` where the operand transitively contains an interface type
// is rejected with E41205.  Covers struct fields (including generic
// instantiations), `Optional<IFoo>`, arrays of interfaces, and
// interface conjunctions (`IFoo & IBar`).
//
// Reverse direction (result type contains interface) lives in
// bit-cast-interface-reverse-wrapped.slang because the reverse path
// is caught at IR layer and requires its own compilation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0" -conformance "FooA:IBar=1"

[anyValueSize(16)]
interface IFoo
{
    float eval();
}

interface IBar
{
    int bar();
}

struct FooA : IFoo, IBar
{
    float x;
    float eval() { return x; }
    int bar() { return 7; }
}

struct Wrapper
{
    IFoo foo;
    float extra;
}

struct Generic<T>
{
    T value;
    int pad;
}

struct RawData { uint data[12]; }

void testStructField(Wrapper w)
{
    RawData raw = bit_cast<RawData>(w);
//CHECK:                            ^ bit_cast involving interface type is not supported
//CHECK:                            ^ 'Wrapper' involves an interface type
}

void testGenericField(Generic<IFoo> g)
{
    RawData raw = bit_cast<RawData>(g);
//CHECK:                            ^ bit_cast involving interface type is not supported
//CHECK:                            ^ 'Generic<IFoo>' involves an interface type
}

void testOptional(Optional<IFoo> opt)
{
    RawData raw = bit_cast<RawData>(opt);
//CHECK:                            ^^^ bit_cast involving interface type is not supported
//CHECK:                            ^^^ 'Optional<IFoo>' involves an interface type
}

void testArrayOfInterface(IFoo arr[2])
{
    RawData raw = bit_cast<RawData>(arr);
//CHECK:                            ^^^ bit_cast involving interface type is not supported
//CHECK:                            ^^^ 'IFoo[2]' involves an interface type
}

void testAndType(IFoo & IBar value)
{
    RawData raw = bit_cast<RawData>(value);
//CHECK:                            ^^^^^ bit_cast involving interface type is not supported
//CHECK:                            ^^^^^ 'IFoo & IBar' involves an interface type
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    outputBuffer[0] = 0.0;
}
