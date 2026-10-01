//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// A single callee, and it is one the pass declines to rewrite. The sibling decline tests
// (`cuda-forward-uniform-unprocessed-callee.slang`, `-signature-preserved-callee.slang`) deliberately
// use *two* callees, because the eligibility gate is only consulted from `updateCallSites`, which runs
// for a function the pass did rewrite - so with a gate present, a single declined callee never reaches
// it. That makes this shape the one the two-callee tests cannot express: the whole kernel has nothing
// but a declined callee, and the aggregate must still reach it without a per-thread temporary.

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
// The aggregate reaches the declined callee by value with no per-thread temporary. Nothing is
// rewritten in this kernel, so `processFunc` returns before `updateCallSites` and the forward is
// never attempted - the by-value argument is just ordinary lowering. Pinning the absence of a spill
// here guards the shape against a future edit that starts forwarding unconditionally and would have
// to fold `*&big` back to `big` to keep this line correct.
// CHECK: exported_read([[BIG]],
// CHECK-NOT: Big_0 {{[A-Za-z0-9_]+}} = [[BIG]];
