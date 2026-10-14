//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// A helper that takes a field of the entry-point uniform, not the whole aggregate. The argument is a
// load of `FieldAddress(big, a)`, an immutable address rooted at the forwarded parameter, so the
// field's address is passed through and `big.a` is not copied into a per-thread temporary.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set big.a.v = {1.0, 2.0, 3.0, 4.0}
//TEST_INPUT:set big.b.v = {10.0, 20.0, 30.0, 40.0}

struct Inner
{
    float v[4];
}

struct Big
{
    Inner a;
    Inner b;
}

[noinline]
float readInner(Inner x, uint i)
{
    return x.v[i];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big big,
    uniform RWStructuredBuffer<float> output)
{
    // CHECK_RT: 11.0
    // CHECK_RT: 22.0
    // CHECK_RT: 33.0
    // CHECK_RT: 44.0
    output[tid] = readInner(big.a, tid) + readInner(big.b, tid);
}

// CHECK: __global__ void computeMain(Big_0 [[BIG:[A-Za-z0-9_]+]], RWStructuredBuffer<float>
// CHECK-NOT: Inner_0 {{.*}}= [[BIG]]
// CHECK: readInner_0(&{{.*}}[[BIG]]{{.*}}a_0,
// CHECK: readInner_0(&{{.*}}[[BIG]]{{.*}}b_0,
