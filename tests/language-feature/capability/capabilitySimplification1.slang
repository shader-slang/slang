//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target glsl -emit-spirv-directly -entry computeMain -stage compute  -profile sm_5_0 -ignore-capabilities

// CHECK_IGNORE_CAPS-NOT: error 36107

[numthreads(1, 1, 1)]
// CHECK: ([[# @LINE+1]]): error 36107: {{.*}}computeMain{{.*}}compute{{.*}}glsl
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
    // CHECK: ([[# @LINE+1]]): note: see using of 'WaveMultiPrefixCountBits'
    WaveMultiPrefixCountBits(true, 0);
}