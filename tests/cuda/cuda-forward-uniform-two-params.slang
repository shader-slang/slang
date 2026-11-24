//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// Two distinct forwardable aggregates in one kernel, each passed to its own helper and also read
// inline. Each parameter is retyped and rewritten independently, so each helper receives its own
// parameter's address and each inline read reads its own parameter.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set a.v = {1.0, 2.0, 3.0, 4.0}
//TEST_INPUT:set b.v = {100.0, 200.0, 300.0, 400.0}

struct Big
{
    float v[4];
}

[noinline]
float readA(Big x, uint i)
{
    return x.v[i];
}

[noinline]
float readB(Big x, uint i)
{
    return x.v[i] * 10.0f;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big a,
    uniform Big b,
    uniform RWStructuredBuffer<float> output)
{
    // out[i] = a.v[i] + 10*b.v[i] + a.v[0] + b.v[1]
    //   i0: 1 + 1000 + 1 + 200 = 1202
    //   i1: 2 + 2000 + 1 + 200 = 2203
    //   i2: 3 + 3000 + 1 + 200 = 3204
    //   i3: 4 + 4000 + 1 + 200 = 4205
    // Crossing the two parameters (one shared load) would change every lane, so the numeric result
    // is a genuine discriminator and not just a restatement of the emit checks.
    // CHECK_RT: 1202.0
    // CHECK_RT: 2203.0
    // CHECK_RT: 3204.0
    // CHECK_RT: 4205.0
    output[tid] = readA(a, tid) + readB(b, tid) + a.v[0] + b.v[1];
}

// Both aggregates stay by-value kernel parameters, so both keep the unchanged kernel ABI.
// CHECK: __global__ void computeMain(Big_0 [[A:[A-Za-z0-9_]+]], Big_0 [[B:[A-Za-z0-9_]+]], RWStructuredBuffer<float>

// Neither parameter is copied.
// CHECK-NOT: Big_0 {{.*}}= [[A]]
// CHECK-NOT: Big_0 {{.*}}= [[B]]

// Each helper receives its own parameter's address - not the same one twice.
// CHECK-DAG: readA_0(&[[A]],
// CHECK-DAG: readB_0(&[[B]],

// Each inline read reads its own parameter: `a.v[0]` from A and `b.v[1]` from B.
// CHECK: (&[[A]])->v_0[int(0)]
// CHECK-SAME: (&[[B]])->v_0[int(1)]
