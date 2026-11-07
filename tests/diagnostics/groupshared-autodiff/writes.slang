// A write to part of a thread-group-shared variable inside a function body that autodiff
// rewrites races with other invocations: address elimination turns an element, field, or
// component write into a read-modify-write of the whole variable, so each invocation stores back
// stale copies of the other invocations' parts. Each case below writes part of shared memory in a
// differentiable function without a custom derivative. A whole-variable write and an atomic stay
// in place; they are in silent.slang. A call that takes part of shared memory as an `out` or
// `inout` argument, including a `[mutating]` receiver, writes that part back when it returns, so
// it is reported whatever the callee. A write in a helper's own body is in
// write-in-force-inline-helper.slang.

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
     ^ consider moving this into a function that is not differentiable or has a custom derivative
*/
    return x;
}

[Differentiable]
float compoundElementWrite(uint i, float x)
{
    g[i] += 1.0;
/*CHECK:
     ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
     ^ consider moving this into a function that is not differentiable or has a custom derivative
*/
    return x;
}

[Differentiable]
float incrementElement(uint i, float x)
{
    g[i]++;
/*CHECK:
     ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
     ^ consider moving this into a function that is not differentiable or has a custom derivative
*/
    return x;
}

[Differentiable]
float fieldWrite(float x)
{
    gPair.b = 1.0;
//CHECK:  ^ writing part of thread-group-shared variable 'gPair' introduces a race condition when this function is differentiated
//CHECK:  ^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float vectorIndexWrite(uint i, float x)
{
    gVector[i] = 1.0;
//CHECK:   ^ writing part of thread-group-shared variable 'gVector' introduces a race condition when this function is differentiated
//CHECK:   ^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float swizzleWrite(float x)
{
    gVector.yz = float2(1.0, 2.0);
//CHECK:    ^^ writing part of thread-group-shared variable 'gVector' introduces a race condition when this function is differentiated
//CHECK:    ^^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float nestedWrite(uint i, float x)
{
    gNested.pairs[i % 2].a = 1.0;
//CHECK:                 ^ writing part of thread-group-shared variable 'gNested' introduces a race condition when this function is differentiated
//CHECK:                 ^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

void setElement(inout float v)
//CHECK:        ^^^^^ 'inout' requires exclusive access
{
    v = 1.0;
}

[ForceInline]
void setElementInlined(inout float v)
//CHECK:               ^^^^^ 'inout' requires exclusive access
{
    v = 1.0;
}

void setElementUint(inout uint v)
//CHECK:            ^^^^^ 'inout' requires exclusive access
{
    v = 1;
}

struct Counter
{
    float n;

    [mutating]
/*CHECK:
     ^^^^^^^^ '[mutating]' requires exclusive access to 'this'
*/
    void bump()
    {
        n += 1.0;
    }
}

groupshared int gInt[4];
groupshared Counter gCounters[4];

[Differentiable]
float callWrite(uint i, float x)
{
    setElement(g[i]);
//CHECK:        ^ 'setElement' needs exclusive access to parameter 'v', but 'g[i]' is thread-group-shared memory that other invocations could access
//CHECK:        ^ consider copying value through a local variable to make the access explicit
//CHECK:        ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
//CHECK:        ^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float inlinedCallWrite(uint i, float x)
{
    setElementInlined(g[i]);
//CHECK:               ^ 'setElementInlined' needs exclusive access to parameter 'v', but 'g[i]' is thread-group-shared memory that other invocations could access
//CHECK:               ^ consider copying value through a local variable to make the access explicit
//CHECK:               ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
//CHECK:               ^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float convertedCallWrite(uint i, float x)
{
    setElementUint(gInt[i]);
//CHECK:               ^ 'setElementUint' needs exclusive access to parameter 'v', but 'gInt[i]' is thread-group-shared memory that other invocations could access
//CHECK:               ^ consider copying value through a local variable to make the access explicit
//CHECK:               ^ writing part of thread-group-shared variable 'gInt' introduces a race condition when this function is differentiated
//CHECK:               ^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float mutatingReceiverWrite(uint i, float x)
{
    gCounters[i].bump();
//CHECK:     ^ 'bump' needs exclusive access to parameter 'this', but 'gCounters[i]' is thread-group-shared memory that other invocations could access
//CHECK:     ^ consider copying value through a local variable to make the access explicit
//CHECK:     ^ writing part of thread-group-shared variable 'gCounters' introduces a race condition when this function is differentiated
//CHECK:     ^ consider moving this into a function that is not differentiable or has a custom derivative
    return x;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    bwd_diff(elementWrite)(i, x, 1.0);
    outputBuffer[i] = x.d + compoundElementWrite(i, 1.0) + incrementElement(i, 1.0) +
                      fieldWrite(1.0) + vectorIndexWrite(i, 1.0) + swizzleWrite(1.0) +
                      nestedWrite(i, 1.0) + callWrite(i, 1.0) + inlinedCallWrite(i, 1.0) +
                      convertedCallWrite(i, 1.0) + mutatingReceiverWrite(i, 1.0);
}
