// Regression test for issue #11933: `-fp-mode precise` was a silent no-op on the
// direct SPIR-V backend -- FP arithmetic was emitted with no `NoContraction`
// decoration, leaving Vulkan drivers free to FMA-fuse / reorder it. `precise`
// must decorate the FP arithmetic with `NoContraction`; `fast`/`default` must not
// emit it (the decoration path is gated strictly on precise mode, so their output
// is unchanged by the fix).

//TEST:SIMPLE(filecheck=PRECISE): -target spirv-asm -entry computeMain -stage compute -fp-mode precise
//TEST:SIMPLE(filecheck=FAST): -target spirv-asm -entry computeMain -stage compute -fp-mode fast
//TEST:SIMPLE(filecheck=DEFAULT): -target spirv-asm -entry computeMain -stage compute -fp-mode default

StructuredBuffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float x = inputBuffer[tid.x];
    // Two FP adds and two FP multiplies on a value loaded from a buffer, so the
    // arithmetic cannot be constant-folded away.
    x = (x + 1.25f) * 0.8125f;
    x = (x + 2.5f) * 0.515625f;
    outputBuffer[tid.x] = x;
}

// Precise mode: the FP arithmetic must carry NoContraction.
//PRECISE: OpDecorate %{{.*}} NoContraction

// Fast and default modes must not introduce NoContraction.
//FAST-NOT: NoContraction
//DEFAULT-NOT: NoContraction
