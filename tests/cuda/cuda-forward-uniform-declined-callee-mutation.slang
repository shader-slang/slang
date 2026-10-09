//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// The `[CudaDeviceExport]` helper writes to its by-value parameter. It must receive a private copy,
// so the write cannot reach the kernel parameter that the rewritten `normal_read` reads through a
// pointer. The runtime check re-reads `big.v[0]` after the call to catch a leaked write.

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
float exported_mutate(Big b, uint i)
{
    b.v[0] = 999.0f;
    return b.v[i];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big big,
    uniform RWStructuredBuffer<float> output)
{
    // The trailing `big.v[0]` re-read is the discriminator. `exported_mutate` writes v[0]=999 to
    // whatever storage it receives; if that storage were the shared kernel parameter rather than a
    // private copy, this re-read would return 999 instead of the original 10.
    //   i0: exported_mutate returns its clobbered copy.v[0]=999, + normal_read 10 + orig 10 => 1019
    //   i1..i3: returns unclobbered copy.v[i], + normal_read v[i] + orig 10 => 50, 70, 90
    // CHECK_RT: 1019.0
    // CHECK_RT: 50.0
    // CHECK_RT: 70.0
    // CHECK_RT: 90.0
    output[tid] = normal_read(big, tid) + exported_mutate(big, tid) + big.v[0];
}

// CHECK: {{.*}}float normal_read_0(Big_0 * {{[A-Za-z0-9_]+}}, uint

// The declined callee keeps its externally visible by-value signature and writes only to a local copy
// of the parameter, so its store cannot reach the caller's storage whatever it is handed.
// CHECK: {{.*}}float exported_mutate(Big_0 [[BV:[A-Za-z0-9_]+]], uint
// CHECK: Big_0 [[LOCAL:[A-Za-z0-9_]+]] = [[BV]];
// CHECK: (&[[LOCAL]])->v_0[int(0)] = 999.0f;

// CHECK: __global__ void computeMain(Big_0 [[BIG:[A-Za-z0-9_]+]], RWStructuredBuffer<float>
// The rewritten callee receives the parameter's address, with no per-thread copy, even though
// another callee keeps its by-value signature.
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]
// CHECK: normal_read_0(&[[BIG]],
// The declined callee receives a value. `exported_mutate(&[[BIG]],` here would be the miscompile.
// CHECK: exported_mutate([[BIG]],
