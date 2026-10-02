//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// The forward inserts exactly ONE load of the retyped parameter, at the top of the entry block, and
// redirects every value read to it. That is only sound because the entry block's first ordinary inst
// dominates every other ordinary inst in the function - so the load must be hoisted, not placed at
// the use. Here the inline value read sits inside a conditional block, which the sibling tests do not
// exercise: they read the aggregate straight-line in the entry block, where any placement dominates
// and a mis-placed load would still pass.

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
        r += data.v[0]; // inline value read in a nested block, dominated by the hoisted load
    output[tid] = r;
}

// Kernel ABI unchanged: still a by-value aggregate parameter.
// CHECK: __global__ void computeMain(Big_0 [[BIG:data_0]], RWStructuredBuffer<float>

// The single load is hoisted into the entry block - before the branch, not inside it - so it
// dominates the conditional read. The CHECK order is the assertion: a load sunk to the use site
// would appear after the `if`, not before it.
// CHECK: Big_0 [[LOADED:[A-Za-z0-9_]+]] = [[BIG]];
// CHECK: readsum_0(&[[BIG]]
// CHECK: if{{ ?}}(
// CHECK: [[LOADED]].v_0
