// GPU-gated companion that NVRTC-compiles and runs the *mutation* path: a [noinline] helper that
// "mutates" its by-value aggregate. The callee copies the pointee (`*b`) into a local and writes
// that, so the forwarded address is only read; a forward that let a write reach the parameter's
// storage (or otherwise miscompiled) fails here, not just in a source-emit FileCheck. Companions:
// cuda-forward-uniform-mutated-copy.slang (emit shape), -forward-compute.slang (read-only). #11774.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set data.values = {1.0, 2.0, 3.0, 4.0}

struct Data
{
    float values[4];
}

// Mutates its by-value copy, then reads it back. Slang copies the pointee into a local before the
// write, so the forwarded address is only read; the write lands on the local.
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
