//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target glsl -emit-spirv-directly -entry computeMain -stage compute  -profile sm_5_0 -ignore-capabilities

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