//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target glsl -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS,exit-code=-1): -target glsl -emit-spirv-directly -entry computeMain -stage compute  -profile sm_5_0 -ignore-capabilities

// With -ignore-capabilities the capability error is suppressed. The unsupported wave intrinsic
// then has no GLSL implementation, so its body ends without a return, which is an error for GLSL.
// CHECK_IGNORE_CAPS-NOT: E36107
// CHECK_IGNORE_CAPS: error[E41009]: non-void function must return
// CHECK_IGNORE_CAPS-NOT: E36107

[numthreads(1, 1, 1)]
// CHECK: error[E36107]
// CHECK: computeMain
void computeMain()
{
    nestedCall();
}

void nestedCall()
{
    nestedSafeCall();
    nestedBadCall();
}

void nestedSafeCall()
{
    AllMemoryBarrier();
}

void nestedBadCall()
{
    WaveMultiPrefixCountBits(true, 0);
}