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
     ^ move the access into a function that is not differentiable or has a custom derivative
*/
    return x;
}

[Differentiable]
float compoundElementWrite(uint i, float x)
{
    g[i] += 1.0;
/*CHECK:
     ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
     ^ move the access into a function that is not differentiable or has a custom derivative
*/
    return x;
}

[Differentiable]
float incrementElement(uint i, float x)
{
    g[i]++;
/*CHECK:
     ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
     ^ move the access into a function that is not differentiable or has a custom derivative
*/
    return x;
}

[Differentiable]
float fieldWrite(float x)
{
    gPair.b = 1.0;
//CHECK:  ^ writing part of thread-group-shared variable 'gPair' introduces a race condition when this function is differentiated
//CHECK:  ^ move the access into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float vectorIndexWrite(uint i, float x)
{
    gVector[i] = 1.0;
//CHECK:   ^ writing part of thread-group-shared variable 'gVector' introduces a race condition when this function is differentiated
//CHECK:   ^ move the access into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float swizzleWrite(float x)
{
    gVector.yz = float2(1.0, 2.0);
//CHECK:    ^^ writing part of thread-group-shared variable 'gVector' introduces a race condition when this function is differentiated
//CHECK:    ^^ move the access into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float nestedWrite(uint i, float x)
{
    gNested.pairs[i % 2].a = 1.0;
//CHECK:                 ^ writing part of thread-group-shared variable 'gNested' introduces a race condition when this function is differentiated
//CHECK:                 ^ move the access into a function that is not differentiable or has a custom derivative
    return x;
}

void setElement(inout float v)
{
    v = 1.0;
}

[ForceInline]
void setElementInlined(inout float v)
{
    v = 1.0;
}

void setElementUint(inout uint v)
{
    v = 1;
}

struct Counter
{
    float n;

    [mutating]
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
//CHECK:        ^ 'v' is an 'out' or 'inout' parameter, which needs exclusive access to its argument during the call, but other invocations in the thread group can access this thread-group-shared memory
//CHECK:        ^ if no other invocation accesses this memory during the call, copy it through a local variable to make that explicit; if the callee needs to see other invocations' writes, declare the parameter 'groupshared'
//CHECK:        ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
//CHECK:        ^ the call writes this argument back when it returns; move the call into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float inlinedCallWrite(uint i, float x)
{
    setElementInlined(g[i]);
//CHECK:               ^ 'v' is an 'out' or 'inout' parameter, which needs exclusive access to its argument during the call, but other invocations in the thread group can access this thread-group-shared memory
//CHECK:               ^ if no other invocation accesses this memory during the call, copy it through a local variable to make that explicit; if the callee needs to see other invocations' writes, declare the parameter 'groupshared'
//CHECK:               ^ writing part of thread-group-shared variable 'g' introduces a race condition when this function is differentiated
//CHECK:               ^ the call writes this argument back when it returns; move the call into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float convertedCallWrite(uint i, float x)
{
    setElementUint(gInt[i]);
//CHECK:               ^ 'v' is an 'out' or 'inout' parameter, which needs exclusive access to its argument during the call, but other invocations in the thread group can access this thread-group-shared memory
//CHECK:               ^ if no other invocation accesses this memory during the call, copy it through a local variable to make that explicit; if the callee needs to see other invocations' writes, declare the parameter 'groupshared'
//CHECK:               ^ writing part of thread-group-shared variable 'gInt' introduces a race condition when this function is differentiated
//CHECK:               ^ the call writes this argument back when it returns; move the call into a function that is not differentiable or has a custom derivative
    return x;
}

[Differentiable]
float mutatingReceiverWrite(uint i, float x)
{
    gCounters[i].bump();
//CHECK:     ^ 'this' is an 'out' or 'inout' parameter, which needs exclusive access to its argument during the call, but other invocations in the thread group can access this thread-group-shared memory
//CHECK:     ^ if no other invocation accesses this memory during the call, copy it through a local variable to make that explicit
//CHECK:     ^ writing part of thread-group-shared variable 'gCounters' introduces a race condition when this function is differentiated
//CHECK:     ^ the call writes this argument back when it returns; move the call into a function that is not differentiable or has a custom derivative
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
