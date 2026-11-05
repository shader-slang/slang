//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// A uniform struct passed to a helper that takes it by borrow is passed by address, and Slang
// emits no per-thread copy of it. SIMPLE pins the emitted CUDA, and COMPARE_COMPUTE runs the
// kernel under NVRTC on a GPU runner and checks the result.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set data.v = {10.0, 20.0, 30.0, 40.0}

struct Big
{
    float v[4];
}

// [noinline] keeps the kernel->helper boundary so the struct is forwarded, not inlined.
[noinline]
float readsum(Big b, uint i)
{
    return b.v[i] + b.v[(i + 1) % 4];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big data,
    uniform RWStructuredBuffer<float> output)
{
    // v = [10,20,30,40]; readsum(i) = v[i] + v[(i+1)%4] = [30,50,70,50].
    // CHECK_RT: 30.0
    // CHECK_RT: 50.0
    // CHECK_RT: 70.0
    // CHECK_RT: 50.0
    output[tid] = readsum(data, tid);
}

// CHECK: __device__ {{.*}} readsum_0(Big_0 *{{.*}})
// CHECK: __global__ void computeMain(Big_0 [[BIG:data_0]], RWStructuredBuffer<float>

// No whole-struct copy spilled before the forward; anchored on the type so reformatted spills trip it.
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]
// CHECK: readsum_0(&[[BIG]]
