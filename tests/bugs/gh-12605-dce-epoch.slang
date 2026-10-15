// Guards the DCE liveness-marking rewrite in shader-slang/slang#12605, which marks live insts by
// stamping `scratchData` with a per-iteration `liveEpoch` (live iff `scratchData == liveEpoch`)
// instead of re-zeroing the subtree each fixpoint iteration. The rewrite must stay
// behavior-preserving in both directions, so this codegen test (no GPU needed) asserts both:
//   - dead code is still removed — `deadMark()` emits an invalid identifier, so if a false-live
//     regression (e.g. a missing `++liveEpoch`) left the unused `arr` loop in, `CHECK-NOT` fails;
//   - live code still survives — the `live` accumulator must reach the output store (`CHECK`),
//     so a dropped-live regression fails too.

//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECKHLSL): -target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Emitting a call to this produces an invalid identifier that would break downstream compilation,
// so DCE must remove it.
__target_intrinsic(cpp, "DEADMARK")
__target_intrinsic(hlsl, "DEADMARK")
[__readNone]
int deadMark() { return 0; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int live = 0;

    // `arr` is written but never read, so DCE removes the whole loop (marker included).
    int arr[8];
    for (int i = 0; i < 8; ++i)
    {
        arr[i] = i + deadMark();
        live += 1;
    }

    outputBuffer[0] = live;

    // CHECK: _computeMain
    // CHECK-NOT: DEADMARK
    // CHECK: outputBuffer{{.*}}live

    // CHECKHLSL: void computeMain
    // CHECKHLSL-NOT: DEADMARK
    // CHECKHLSL: outputBuffer{{.*}}live
}
