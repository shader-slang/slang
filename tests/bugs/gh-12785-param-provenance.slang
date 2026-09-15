// Regression test for shader-slang/slang#12785 (obscured-provenance resource reads are not
// commoned — the deliberate conservative boundary of the repeatable-read gate).
//
// The repeatable-read gate only commons a dominated identical call when it can *prove* the callee
// reads exclusively unqualified read-only memory. It proves this by peeling each read's access
// chain to its root and inspecting that root's `globallycoherent`/`volatile` qualifier. When the
// root is a module-scope global that inspection is exact. When the buffer instead arrives through a
// function parameter (as here), a phi, a `select`, or a handle loaded from memory not provably
// immutable, the qualifier is not propagated across that edge (a handle loaded from provably
// immutable memory IS followed), so the gate cannot prove the resource is unqualified and
// conservatively treats the read as non-repeatable. Recovering this coverage needs qualifier
// propagation across param/phi, which is a separate representation fix.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

StructuredBuffer<uint> inputData;
RWStructuredBuffer<uint> outputData;

// `buf` is a parameter: its provenance (and any qualifier on the underlying resource) is obscured,
// so the gate cannot classify this wrapper as a repeatable read-only access.
[noinline]
uint loadFrom(StructuredBuffer<uint> buf, uint index)
{
    return buf[index];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadFrom(inputData, index);
    uint b = loadFrom(inputData, index);
    outputData[index] = a + b;
}
// Both calls survive (a repeatable read-only access could not be proven for the parameter-sourced
// buffer). Anchor on the call, which passes `inputData` as arg 0, so the emitted definition
// `loadFrom_0(StructuredBuffer...` is NOT counted; if the two calls were wrongly commoned only one
// would remain and the count-of-two assertion below would fail.
// CHECK-COUNT-2: loadFrom_0(inputData
// CHECK-NOT: loadFrom_0(inputData
