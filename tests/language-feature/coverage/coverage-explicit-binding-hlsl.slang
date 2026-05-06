// Verifies that `-trace-coverage-binding <index> <space>` steers the
// synthesized `__slang_coverage` buffer to a user-specified slot on
// the HLSL target. Without the option, parameter binding auto-allocates
// the slot, which is fine for hosts that read the assignment back via
// the metadata API. The option exists for hosts that need the slot
// fixed at compile time (for example to pre-build a D3D12 root
// signature).
//
// HLSL emit literally passes the `register(uN, spaceM)` clause through
// to the output, so a filecheck on the emitted HLSL is enough to
// confirm the steer reached the target.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -trace-coverage-binding 7 3 -profile cs_5_0

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        int x = 1 + 2;
    }
}

// The synthesized buffer (`__slang_coverage`, mangled to `_slang_coverage_<n>`
// in HLSL emit) should appear with the explicit (u7, space3) register
// clause. If the option is dropped, this would regress to the
// auto-allocated slot (typically u0).
//CHECK: _slang_coverage{{.*}}register(u7, space3)
