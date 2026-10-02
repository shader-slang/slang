//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// The only helper is one the pass does not rewrite, so no call site is rebuilt and the aggregate
// reaches it by value, as on master, with no per-thread temporary.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set big.v = {10.0, 20.0, 30.0, 40.0}

struct Big
{
    float v[4];
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
    // CHECK_RT: 10.0
    // CHECK_RT: 20.0
    // CHECK_RT: 30.0
    // CHECK_RT: 40.0
    output[tid] = exported_read(big, tid);
}

// The declined callee keeps its externally visible by-value signature.
// CHECK: {{.*}}float exported_read(Big_0 {{[A-Za-z0-9_]+}}, uint

// CHECK: __global__ void computeMain(Big_0 [[BIG:[A-Za-z0-9_]+]], RWStructuredBuffer<float>
// CHECK: exported_read([[BIG]],
// CHECK-NOT: Big_0 {{[A-Za-z0-9_]+}} = [[BIG]];
