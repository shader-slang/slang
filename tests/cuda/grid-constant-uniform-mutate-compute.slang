// GPU-gated companion that NVRTC-compiles the *mutation* path: a helper that writes through its
// by-value aggregate parameter. This exercises the most safety-critical scenario — the emitter
// stripping `const` via `const_cast<Data_0*>(&data)` on a `__grid_constant__` object whose local
// copy inside the callee is then written — and verifies the numeric result, so a `const_cast` that
// let a write reach grid-constant memory (or otherwise miscompiled) would fail here rather than
// only in a source-emit FileCheck. Companion to grid-constant-uniform-mutated-copy.slang (which
// checks emit shape) and grid-constant-uniform-forward-compute.slang (read-only forward). Fix #11774.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set data.values = {1.0, 2.0, 3.0, 4.0}

struct Data
{
    float values[4];
}

// Mutates its by-value copy, then reads it back. Slang copies the pointee into a local before the
// write, so the forwarded grid-constant address is only read; the write lands on the local.
[noinline]
float bump(Data d, uint i)
{
    d.values[i] = d.values[i] + 100.0f;
    return d.values[i];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID, uniform Data data, uniform RWStructuredBuffer<float> output)
{
    // values = [1,2,3,4]; output[i] = values[i] + 100 = [101,102,103,104].
    // CHECK: 101.0
    // CHECK: 102.0
    // CHECK: 103.0
    // CHECK: 104.0
    output[tid] = bump(data, tid);
}
