// GPU-gated companion to the source-emit forward-uniform tests: compiles the emitted CUDA under
// NVRTC and runs it, checking the numeric result, so a malformed address-forward the FileCheck
// tests cannot catch fails here. Forwards a by-value uniform aggregate into a [noinline] helper
// (`(&(data))`) rather than reading it inline. Fix for #11774.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set data.values = {10.0, 20.0, 30.0, 40.0}
//TEST_INPUT:set data.indices = {3, 2, 1, 0}

struct Data
{
    float values[4];
    int indices[4];
}

// [noinline] keeps the kernel->helper boundary intact so the uniform aggregate's address is
// forwarded into a borrow-in pointer parameter instead of being inlined away.
[noinline]
float gather(Data d, uint i)
{
    return d.values[d.indices[i]];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID, uniform Data data, uniform RWStructuredBuffer<float> output)
{
    // indices = [3,2,1,0] so output[i] = values[indices[i]] = [40,30,20,10].
    // CHECK: 40.0
    // CHECK: 30.0
    // CHECK: 20.0
    // CHECK: 10.0
    output[tid] = gather(data, tid);
}
