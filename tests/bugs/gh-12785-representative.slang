// Regression test for shader-slang/slang#12785 (representative-tracking across a declined reuse).
//
// After a store forces the second call `b` to decline reuse of the first call `a`, `b` -- not `a` --
// must become the recorded representative for the structural key (the `ReplaceRepresentative`
// action), so a third identical call `c` with nothing between it and `b` commons with `b`. If the
// representative stayed the pre-store `a`, `c` would find no dominating clobber-free occurrence and
// stay distinct, losing this commoning.
//
// Shape: `a` (pre-store) is distinct; `b` and `c` (no store between them) collapse to one call.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

StructuredBuffer<float> inputData;
RWStructuredBuffer<float> outputData;

[noinline]
float loadValue(uint index)
{
    return inputData[index];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    float a = loadValue(i);        // pre-store
    outputData[i] = 7.0f;          // store: blocks a<->b and a<->c
    float b = loadValue(i);        // distinct from a
    float c = loadValue(i);        // nothing between b and c -> commons with b
    outputData[i] = a + b + c;
}

// `a` survives as one call and `b`/`c` collapse to one, so exactly two `loadValue(i...` call sites
// remain (the definition reads `loadValue_0(uint index_0)`, which does not match `loadValue_0(i`).
// CHECK-COUNT-2: loadValue_0(i
// CHECK-NOT: loadValue_0(i
