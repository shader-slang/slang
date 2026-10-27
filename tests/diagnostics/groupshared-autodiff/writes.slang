// A write to part of a thread-group-shared variable inside a function body that autodiff
// rewrites races with other invocations: address elimination turns an element, field, or
// component write into a read-modify-write of the whole variable, so each invocation stores back
// stale copies of the other invocations' parts. Each case below writes part of shared memory in a
// differentiable function without a custom derivative, either directly or through the function's
// own `groupshared` parameter. A whole-variable write and an atomic stay in place; they are in
// silent.slang.
//
// The last case writes through a non-differentiable `[ForceInline]` helper. Such a helper is
// inlined before autodiff, so its write becomes a partial write in the differentiated body, but
// the write is in the helper's body, which a check of the differentiated function does not see.
//
// Disabled: no check diagnoses these writes yet. The annotations match only the severity,
// because the diagnostic does not exist.

//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

RWStructuredBuffer<float> outputBuffer;

struct Pair
{
    float a;
    float b;
}

struct Nested
{
    Pair pairs[2];
}

groupshared float g[4];
groupshared Pair gPair;
groupshared float4 gVector;
groupshared Nested gNested;

[ForceInline]
float bumpInline(__ref float a)
{
    a += 1.0;
    return a;
}

[Differentiable]
float elementWrite(uint i, float x)
{
    g[i] = float(i);
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float compoundElementWrite(uint i, float x)
{
    g[i] += 1.0;
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float incrementElement(uint i, float x)
{
    g[i]++;
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float fieldWrite(float x)
{
    gPair.b = 1.0;
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float vectorIndexWrite(uint i, float x)
{
    gVector[i] = 1.0;
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float swizzleWrite(float x)
{
    gVector.yz = float2(1.0, 2.0);
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float nestedWrite(uint i, float x)
{
    gNested.pairs[i % 2].a = 1.0;
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float writeThroughOwnParameter(no_diff groupshared float a[4], uint i, float x)
{
    a[i] = 1.0;
//CHECK:     ^ warning
    return x;
}

[Differentiable]
float throughForceInlineNonDifferentiable(uint i, float x)
{
    return x * no_diff(bumpInline(g[i]));
//CHECK:                              ^ warning
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    bwd_diff(elementWrite)(i, x, 1.0);
    outputBuffer[i] = x.d + compoundElementWrite(i, 1.0) + incrementElement(i, 1.0) +
                      fieldWrite(1.0) + vectorIndexWrite(i, 1.0) + swizzleWrite(1.0) +
                      nestedWrite(i, 1.0) + writeThroughOwnParameter(g, i, 1.0) +
                      throughForceInlineNonDifferentiable(i, 1.0);
}
