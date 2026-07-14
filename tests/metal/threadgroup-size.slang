// TEST:SIMPLE(filecheck=METAL4): -target metal -capability metallib_4_0
// TEST:SIMPLE(filecheck=METAL3): -target metal -capability metallib_3_1

// When metal4.0 is in effect the emitter uses the metal4.0-only
// `[[required_threads_per_threadgroup]]` attribute, so compiling all the way to a metallib must
// request `-std=metal4.0` for the downstream metal compiler to accept it. Before this was fixed the
// downstream std was hard-coded to metal3.1, so this compile failed on a metal4.0-capable toolchain
// (see shader-slang/slang#11999). This case requires the metal compiler, so it only runs on macOS CI.
// TEST:SIMPLE(filecheck=METALLIB4): -target metallib -stage compute -entry computeMain -capability metallib_4_0

RWStructuredBuffer<int> outputBuffer;

// Ensure that metal4 compilation includes the required_threads_per_threadgroup annotation.
// METAL3-NOT: {{\[\[}}required_threads_per_threadgroup(32, 16, 1){{\]\]}}
// METAL4: {{\[\[}}required_threads_per_threadgroup(32, 16, 1){{\]\]}}

// The metallib compile just needs to succeed; the AIR module disassembles to the entry point.
// METALLIB4: define void @computeMain

[numthreads(32, 16, 1)]
void computeMain(uint3 dtid: SV_DispatchThreadID)
{
    uint idx = dtid.y;
    if (dtid.x == idx)
    {
        outputBuffer[idx] = (int)idx;
    }
}
