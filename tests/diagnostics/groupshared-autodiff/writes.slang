// A write to part of a thread-group-shared variable inside a function body that autodiff
// rewrites races with other invocations: address elimination turns an element, field, or
// component write into a read-modify-write of the whole variable, so each invocation stores back
// stale copies of the other invocations' parts. Each case below writes part of shared memory in a
// differentiable function without a custom derivative, either directly or through the function's
// own `groupshared` parameter. A whole-variable write and an atomic stay in place; they are in
// silent.slang. A write in a `[ForceInline]` helper is in write-in-force-inline-helper.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

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

[Differentiable]
float elementWrite(uint i, float x)
{
    g[i] = float(i);
/*CHECK:
     ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
     ^ move the write into a function that is not differentiable or has custom derivatives
*/
    return x;
}

[Differentiable]
float compoundElementWrite(uint i, float x)
{
    g[i] += 1.0;
/*CHECK:
     ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
     ^ move the write into a function that is not differentiable or has custom derivatives
*/
    return x;
}

[Differentiable]
float incrementElement(uint i, float x)
{
    g[i]++;
/*CHECK:
     ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
     ^ move the write into a function that is not differentiable or has custom derivatives
*/
    return x;
}

[Differentiable]
float fieldWrite(float x)
{
    gPair.b = 1.0;
//CHECK:  ^ writing part of thread-group-shared variable 'gPair' introduces a race condition when this function is differentiated
//CHECK:  ^ move the write into a function that is not differentiable or has custom derivatives
    return x;
}

[Differentiable]
float vectorIndexWrite(uint i, float x)
{
    gVector[i] = 1.0;
//CHECK:   ^ writing part of thread-group-shared variable 'gVector' introduces a race condition when this function is differentiated
//CHECK:   ^ move the write into a function that is not differentiable or has custom derivatives
    return x;
}

[Differentiable]
float swizzleWrite(float x)
{
    gVector.yz = float2(1.0, 2.0);
//CHECK:    ^^ writing part of thread-group-shared variable 'gVector' introduces a race condition when this function is differentiated
//CHECK:    ^^ move the write into a function that is not differentiable or has custom derivatives
    return x;
}

[Differentiable]
float nestedWrite(uint i, float x)
{
    gNested.pairs[i % 2].a = 1.0;
//CHECK:                 ^ writing part of thread-group-shared variable 'gNested' introduces a race condition when this function is differentiated
//CHECK:                 ^ move the write into a function that is not differentiable or has custom derivatives
    return x;
}

[Differentiable]
float writeThroughOwnParameter(no_diff groupshared float a[4], uint i, float x)
{
    a[i] = 1.0;
/*CHECK:
     ^ writing part of thread-group-shared variable 'a' introduces a race condition when this function is differentiated
     ^ move the write into a function that is not differentiable or has custom derivatives
*/
    return x;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    bwd_diff(elementWrite)(i, x, 1.0);
    outputBuffer[i] = x.d + compoundElementWrite(i, 1.0) + incrementElement(i, 1.0) +
                      fieldWrite(1.0) + vectorIndexWrite(i, 1.0) + swizzleWrite(1.0) +
                      nestedWrite(i, 1.0) + writeThroughOwnParameter(g, i, 1.0);
}
