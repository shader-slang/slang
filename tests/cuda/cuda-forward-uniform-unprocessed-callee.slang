//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// One uniform aggregate passed to two helpers: `normal_read`, whose parameter the pass rewrites to a
// pointer, and `exported_read`, which `shouldProcessFunction` leaves alone because
// `[CudaDeviceExport]` gives it an externally visible by-value signature. The forwarded parameter
// reaches the rewritten helper by address and the exported helper by value. Handing `&big` to
// `exported_read` would pass a `Big_0 *` to a by-value `Big_0` parameter, which NVRTC rejects.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set big.v = {10.0, 20.0, 30.0, 40.0}

struct Big
{
    float v[4];
}

[noinline]
float normal_read(Big b, uint i)
{
    return b.v[i];
}

[CudaDeviceExport]
float exported_read(Big b, uint i)
{
    return b.v[i];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big big,
    uniform RWStructuredBuffer<float> output)
{
    // Both helpers read v[i], so each lane gets 2 * v[i].
    // CHECK_RT: 20.0
    // CHECK_RT: 40.0
    // CHECK_RT: 60.0
    // CHECK_RT: 80.0
    output[tid] = normal_read(big, tid) + exported_read(big, tid);
}

// CHECK: {{.*}}float normal_read_0(Big_0 * {{[A-Za-z0-9_]+}}, uint

// `exported_read` keeps its externally visible by-value signature.
// CHECK: {{.*}}float exported_read(Big_0 {{[A-Za-z0-9_]+}}, uint

// CHECK: __global__ void computeMain(Big_0 [[BIG:[A-Za-z0-9_]+]], RWStructuredBuffer<float>
// The rewritten callee receives the parameter's address, with no per-thread copy.
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]
// CHECK: normal_read_0(&[[BIG]],
// And the declined callee must receive a value. `exported_read(&[[BIG]],` here would be the
// miscompile: a `Big_0 *` argument reaching a by-value `Big_0` parameter.
// CHECK: exported_read([[BIG]],
