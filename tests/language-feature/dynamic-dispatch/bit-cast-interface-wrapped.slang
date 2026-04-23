// `bit_cast` where the operand or result transitively contains an
// interface type is also rejected.  Covers struct fields (including
// generic instantiations) and `Optional<IFoo>`.
//
// Complements bit-cast-interface-roundtrip.slang (forward direction,
// bare interface) and bit-cast-interface-serialize.slang (reverse
// direction).  See also serialize-dynamic-object.slang for the
// supported `serializeDynamicObject` / `createDynamicObject` pair.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0"

[anyValueSize(16)]
interface IFoo
{
    float eval();
}

struct FooA : IFoo
{
    float x;
    float eval() { return x; }
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

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Wrapper w;
    w.foo = createDynamicObject<IFoo>(0, 3.0);
    w.extra = 1.0;
    testStructField(w);

    Generic<IFoo> g;
    g.value = createDynamicObject<IFoo>(0, 4.0);
    g.pad = 0;
    testGenericField(g);

    Optional<IFoo> opt = none;
    testOptional(opt);

    outputBuffer[0] = 0.0;
}
