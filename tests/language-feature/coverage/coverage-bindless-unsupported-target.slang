// The bindless coverage form requires descriptor indexing, which the non-Khronos
// coverage targets do not have — CPU and CUDA have no descriptors to index at all.
//
// This must DIAGNOSE rather than quietly fall back to the single-buffer form: a
// host that asked for one shared binding and silently received one binding per
// shader would not find out until its pipeline layout failed to match at runtime,
// which is exactly the class of failure the option exists to remove.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile sm_6_0 -trace-coverage -trace-coverage-bindless 31 3

RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tid.x;
}

// Matched on the code digits and a message fragment rather than on a rendered
// `error E45116` / `error[E45116]` spelling: the two diagnostic styles differ
// in punctuation, and this test is about the rejection happening at all.
//CHECK: 45116
//CHECK: Khronos
