//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// A value read of the forwarded parameter inside a conditional block. Every value use of the
// retyped parameter becomes a load through it at the use site, so the read in the nested block
// reads the kernel parameter in place instead of a whole-struct copy made at the top of the kernel.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set data.v = {10.0, 20.0, 30.0, 40.0}

struct Big
{
    float v[4];
}

// [noinline] keeps the kernel->helper boundary so the aggregate is forwarded, not inlined.
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
    // v = [10,20,30,40]; readsum(i) = v[i] + v[(i+1)%4] = [30,50,70,50]; lanes 1..3 add v[0]=10.
    // CHECK_RT: 30.0
    // CHECK_RT: 60.0
    // CHECK_RT: 80.0
    // CHECK_RT: 60.0
    float r = readsum(data, tid);
    if (tid > 0)
        r += data.v[0];
    output[tid] = r;
}

// Kernel ABI unchanged: still a by-value aggregate parameter.
// CHECK: __global__ void computeMain(Big_0 [[BIG:data_0]], RWStructuredBuffer<float>

// The read in the nested block loads through the parameter in place, so no whole-struct copy is
// made anywhere in the kernel.
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]
// CHECK: readsum_0(&[[BIG]]
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]
// CHECK: if{{ ?}}(
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]
// CHECK: (&[[BIG]])->v_0[int(0)]
