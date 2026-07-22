// GPU-gated companion to cuda-forward-uniform-array.slang: NVRTC-compiles and runs the array
// address-forward, checking the numeric result, so a malformed forward the emit-only FileCheck test
// cannot catch fails here. The by-value uniform aggregate here is a struct wrapping an array; it is
// forwarded into a [noinline] helper (`&data`) that indexes the array, and also read inline
// (`data.v[0]`), exercising both the forward and the load-redirect path. #11774.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set data.v = {10.0, 20.0, 30.0, 40.0}

struct Data
{
    float v[4];
}

// [noinline] keeps the kernel->helper boundary intact so the uniform aggregate's address is
// forwarded into a borrow-in pointer parameter instead of being inlined away.
[noinline]
float gather(Data d, uint i)
{
    return d.v[(i + 1) % 4];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID, uniform Data data, uniform RWStructuredBuffer<float> output)
{
    // gather returns v[(i+1)%4]; plus the inline read v[0] (=10).
    // i=0..3 -> v[1,2,3,0] = [20,30,40,10]; +10 => [30,40,50,20].
    // CHECK: 30.0
    // CHECK: 40.0
    // CHECK: 50.0
    // CHECK: 20.0
    output[tid] = gather(data, tid) + data.v[0];
}
