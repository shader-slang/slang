// Regression test for #14524: a store through a `__getAddress` pointer into an
// `RWStructuredBuffer` must not be removed as dead just because a later store
// overwrites it and the read in between goes through the buffer itself —
// `buf[0]` loads the same element the first store wrote. Expected `buf[3]` is
// 5, the value the intervening read observes; with the dead-store bug it is
// the stale buffer content 2.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -g0 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -g0

//TEST_INPUT:ubuffer(data=[2 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<float> buf;

// The first store is read by the intervening `buf[0]` load before the second
// store overwrites it, so it must stay.
[noinline]
float storeLoadOverwrite(float* p, float v)
{
    *p = v;
    float s = buf[0];
    *p = 2.0f;
    return s;
}

// Same scenario with the read spelled `.Load(0)`, which lowers to a dedicated
// `RWStructuredBufferLoad` instruction instead of a plain load, so the dead
// store scan must treat it as a read of the buffer contents.
[noinline]
float storeLoadOverwriteLoadOp(float* p, float v)
{
    *p = v;
    float s = buf.Load(0);
    *p = 2.0f;
    return s;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float r = storeLoadOverwrite(__getAddress(buf[0]), 5.0f);
    buf[3] = r;
    float r2 = storeLoadOverwriteLoadOp(__getAddress(buf[0]), 7.0f);
    buf[2] = r2;
}

// CHECK: type: float
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 5.000000
