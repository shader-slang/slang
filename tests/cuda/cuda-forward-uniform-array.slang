//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// A by-value uniform array is forwarded by address into a borrow-in helper instead of copied into a
// per-thread temporary (the #11774 slowdown). Array counterpart of the struct-forward test. The
// same shader is checked two ways: SIMPLE pins the emitted CUDA (the forward is present, no whole-
// array copy is spilled), and COMPARE_COMPUTE runs it under NVRTC so a malformed forward the emit
// check cannot catch fails on the numeric result. (The COMPARE_COMPUTE run is GPU-gated - it reports
// `ignored` on a GPU-less runner and executes on the GPU lane.)

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set tensors = {10.0, 20.0, 30.0, 40.0}

// [noinline] keeps the kernel->helper boundary so the array is forwarded, not inlined.
[noinline]
float gather(float tensors[4], uint i)
{
    return tensors[i] + tensors[(i + 1) % 4];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform float tensors[4],
    uniform RWStructuredBuffer<float> output)
{
    // tensors = [10,20,30,40]; out[i] = t[i] + t[(i+1)%4] = [30,50,70,50].
    // CHECK_RT: 30.0
    // CHECK_RT: 50.0
    // CHECK_RT: 70.0
    // CHECK_RT: 50.0
    output[tid] = gather(tensors, tid);
}

// CHECK: gather{{[A-Za-z0-9_]*}}(FixedArray<float, 4>{{.*}}*
// CHECK: __global__ void computeMain(FixedArray<float, 4>{{.*}}[[TENSORS:tensors_[0-9]+]], RWStructuredBuffer<float>

// No whole-array copy spilled before the forward; anchored on the type so reformatted spills trip it.
// CHECK-NOT: FixedArray<float, 4>{{.*}}= [[TENSORS]]
// CHECK: gather{{[A-Za-z0-9_]*}}(&[[TENSORS]]
