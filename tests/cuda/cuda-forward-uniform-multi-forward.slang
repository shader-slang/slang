//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// The same uniform aggregate passed to two rewritten helpers and also read inline. Both helpers
// receive the parameter's address and the inline read reads the parameter in place; no
// whole-struct copy is made.
//
// The same shader is also run under NVRTC (COMPARE_COMPUTE), so a wrong address or a pointer read
// as a value fails on the numeric result. (GPU-gated: `ignored` on a GPU-less runner.)

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set big.v = {10.0, 20.0, 30.0, 40.0}

struct Big
{
    float v[4];
}

// [noinline] keeps each kernel->helper boundary so the aggregate is forwarded, not inlined.
[noinline]
float readsum(Big b, uint i)
{
    return b.v[i] + b.v[(i + 1) % 4];
}

[noinline]
float readmax(Big b, uint i)
{
    return b.v[i] > b.v[(i + 1) % 4] ? b.v[i] : b.v[(i + 1) % 4];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big big,
    uniform RWStructuredBuffer<float> output)
{
    // v = [10,20,30,40]; out[i] = readsum(i) + readmax(i) + v[0]:
    // i0: 30 + 20 + 10 = 60; i1: 50 + 30 + 10 = 90; i2: 70 + 40 + 10 = 120; i3: 50 + 40 + 10 = 100.
    // CHECK_RT: 60.0
    // CHECK_RT: 90.0
    // CHECK_RT: 120.0
    // CHECK_RT: 100.0
    output[tid] = readsum(big, tid) + readmax(big, tid) + big.v[0];
}

// Kernel ABI unchanged: the aggregate is still a plain by-value kernel argument.
// CHECK: __global__ void computeMain(Big_0 [[BIG:big_[0-9]*]], RWStructuredBuffer<float>

// No whole-struct copy is made for either helper or for the inline read.
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]

// CHECK-DAG: readsum_0(&[[BIG]]
// CHECK-DAG: readmax_0(&[[BIG]]
// CHECK: (&[[BIG]])->v_0[int(0)]
