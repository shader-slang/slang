// Companion to gh-13260: when a non-receiver static requirement has NO default to fall back to,
// the fix must report a clean unsatisfied-requirement diagnostic rather than adapting a same-name
// instance method (which previously bound the argument as `this` and miscompiled / asserted at
// slang-ir.cpp:5822).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct Packed { uint instance; float2 uv; }

interface IHit2
{
    static This unpack(Packed p);
    float value();
    static float value(Packed p);
//CHECK:         ^^^^^ see interface requirement declaration of 'value'
}

struct Hit2 : IHit2
{
    uint instance;
    float2 uv;
    static This unpack(Packed p) { return {p.instance, p.uv}; }
    float value() { return float(instance) + uv.x + uv.y; }
//CHECK:  ^^^^^ interface requirement mismatch
//CHECK:  ^^^^^ member 'value' does not match interface requirement.
}

float callStatic<T : IHit2>(Packed p)
{
    return T.value(p);
}

RWStructuredBuffer<float> result;

[numthreads(1, 1, 1)]
void computeMain()
{
    Packed p;
    p.instance = 1;
    p.uv = float2(0, 0);
    result[0] = callStatic<Hit2>(p);
}
