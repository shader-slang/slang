//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
//META: purpose=Every function using the wave active mask gains an explicit mask parameter while entry points compute the mask as the first operation of their body, so the emitted CUDA gains both an extra argument and a mask computation.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the active-mask synthesis footprint on the
// target that needs it. `helper` is a non-entry-point function using a
// wave intrinsic, so the pass has to thread a mask into it rather than
// letting it query one; `main` is the entry point, so the mask is computed
// there instead. The CHECKs assert `helper` gained a trailing `uint`
// parameter and that the kernel body opens with the mask computation that
// feeds it.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

RWStructuredBuffer<int> outBuf;

int helper(int x)
{
    return int(WaveActiveSum(x));
}

[numthreads(32, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = helper(int(tid.x));
}

// The helper carries the mask as an extra parameter ...
// CUDA: helper_{{[0-9]+}}(int {{[A-Za-z0-9_]+}}, uint
// ... and the kernel computes the mask before calling it.
// CUDA: __ballot_sync(
