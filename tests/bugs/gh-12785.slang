// Regression test for shader-slang/slang#12785 (positive, adjacent calls).
//
// A `[noinline]` wrapper whose body performs only a read-only `StructuredBuffer` load participates
// in dominance-preserving CSE: two identical calls with no memory-writing/synchronizing instruction
// between them collapse to a single call, just like a pure (readNone) wrapper. This must NOT make
// the call movable/speculatable across control flow; that invariant is covered by
// tests/bugs/gh-3429.slang and by gh-12785-negative.slang / gh-12785-wave.slang.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

StructuredBuffer<float> inputData;
RWStructuredBuffer<float> outputData;

[noinline]
float loadValue(uint index)
{
    return inputData[index];
}

[noinline]
float computeValue(float value)
{
    return value * value + 1.0f;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;

    // Control: an inferred-readnone function is commoned (one emitted call).
    float pureA = computeValue(float(index));
    float pureB = computeValue(float(index));

    // Both identical calls must common to a single emitted call.
    float loadedA = loadValue(index);
    float loadedB = loadValue(index);

    outputData[index] = pureA + pureB + loadedA + loadedB;
}

// The wrapper is *called* exactly once. `loadValue_0(index...` matches only call sites (the
// definition reads `loadValue_0(uint index_0)`), so a single match confirms the two calls were
// commoned into one.
// CHECK-COUNT-1: loadValue_0(index
// CHECK-NOT: loadValue_0(index
