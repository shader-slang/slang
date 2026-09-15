// Regression test for shader-slang/slang#12785 (negative, backward-substitution).
//
// This pins the backward-substitution hazard in `deduplicate`: it records the most-recent
// representative per structural key and recurses over operands, so after a declined reuse (a store
// between two identical calls) the map holds the LATER call, and the operand walk of the enclosing
// expression then presents the EARLIER call as the candidate against that later representative. A
// block-level dominance guard would be reflexive within a block and accept that backward pair,
// rewriting the earlier (pre-store) call to the later (post-store) one; the instruction-level guard
// enforced here does not.
//
// A helper returns `before + 1000 * after`, so both calls are operands of one add that redundancy
// removal visits -- the exact operand-recursion path. With a store between the two calls, the two
// calls must remain distinct: neither a forward CSE across the store nor a backward rewrite of
// `before` to `after` is permitted.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

StructuredBuffer<float> inputData;
RWStructuredBuffer<float> outputData;

[noinline]
float loadValue(uint index)
{
    return inputData[index];
}

// `[noinline]` keeps this a stable function boundary so the two inner calls reliably emit as
// `loadValue_0(index_0)` (the anchor below) rather than inlining into `computeMain` with a
// rewritten argument name.
[noinline]
float encodeBeforeAfter(uint index)
{
    float before = loadValue(index);   // reads inputData[index]
    outputData[index] = 100.0f;        // store between (SRV/UAV may alias the same memory)
    float after = loadValue(index);    // must stay distinct from `before`
    return before + 1000.0f * after;   // both calls feed one movable add
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputData[dispatchThreadID.x] = encodeBeforeAfter(dispatchThreadID.x);
}

// Both calls must survive; a backward/forward CSE across the store would leave one.
// CHECK-COUNT-2: loadValue_0(index
