// Companion to gh-13260. A static requirement whose first parameter is not the conforming (`This`)
// type is not satisfied by a same-name instance method: binding the argument as the callee's `this`
// would call the instance method on an unrelated value (in gh-13260 this became an invalid forwarder
// and an internal assert in `emitFieldAddress`). With no interface default to fall back to,
// conformance fails with a diagnostic (E38105, reported on the excluded instance `value()`) rather
// than miscompiling.

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
//CHECK:  ^^^^^ E38105
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
