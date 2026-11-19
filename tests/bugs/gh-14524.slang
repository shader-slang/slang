// Regression test for #14524: a store through a `__getAddress` pointer into an
// `RWStructuredBuffer` must not be removed as dead just because a later store
// overwrites it and the read in between goes through the buffer itself —
// `buf[0]` loads the same element the first store wrote. Expected `buf[3]` is
// 5 and `buf[2]` is 7, the values the intervening reads observe; with the
// dead-store bug they are the stale buffer content 2.
//
// All three lanes pin `-g0`: at higher debug-info levels the store is kept
// alive by debug-variable materialization, which would make this test pass
// vacuously. The vk/cuda rows forward the flag to the inner slang invocation
// with `-Xslang`; the cpu row talks to slangc directly.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -g0 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -g0

//TEST_INPUT:ubuffer(data=[2.0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<float> buf;

// The intervening `buf[0]` is a plain load of a resource-content address, so
// this function pins the class-level rule in `canAddrClassesAlias`: a user
// pointer may alias a bound resource.
[noinline]
float storeLoadOverwrite(float* p, float v)
{
    *p = v;
    float s = buf[0];
    *p = 2.0f;
    return s;
}

// Same scenario with the read spelled `.Load(0)`, which lowers to a dedicated
// `RWStructuredBufferLoad` instruction instead of a plain load, so this
// function pins the resource-content read check in
// `canInstHaveSideEffectAtAddress`'s default arm.
[noinline]
float storeLoadOverwriteLoadOp(float* p, float v)
{
    *p = v;
    float s = buf.Load(0);
    *p = 2.0f;
    return s;
}

// The class rule that keeps the stores above also fixes load forwarding: a
// store into the memory `p` points to must stop `*p` from being forwarded to
// its earlier value, because `p` may alias the buffer element that was just
// overwritten. Expected 3, the overwritten value; with the old class rule the
// load forwarded to 5 and the stale value won.
[noinline]
float forwardAcrossBufferStore(float* p)
{
    *p = 5.0f;
    buf[1] = 3.0f;
    return *p;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float r = storeLoadOverwrite(__getAddress(buf[0]), 5.0f);
    buf[3] = r;
    float r2 = storeLoadOverwriteLoadOp(__getAddress(buf[0]), 7.0f);
    buf[2] = r2;
    buf[4] = forwardAcrossBufferStore(__getAddress(buf[1]));
}

// CHECK: type: float
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 3.000000
