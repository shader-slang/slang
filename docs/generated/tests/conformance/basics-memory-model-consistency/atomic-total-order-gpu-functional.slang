//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#atomics
//META: doc_section_digest=a51e4f61c144b6b055f62cb6dd435450ea8054e86989ebdfc212ed4b29838fc8
//META: purpose=All atomic modifications of a single variable occur in a total order: two concurrent `InterlockedAdd` increments on a 0-initialized variable always result in a final value of 3 (the modifications are serialized).
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "All atomic modifications of a single variable occur in a total
// order. That is, the modifications are serialized. For example, if one thread
// increments a 0-initialized atomic variable by 1 and another thread increments it
// by 2, all threads will observe the increments in the same order: either 0->1->3
// or 0->2->3."
// This test dispatches 64 threads each incrementing by 1 and verifies the final
// sum is exactly 64, proving all increments are serialized without loss.
// GPU-only (CI nightly); not executable locally without a GPU.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type

RWStructuredBuffer<uint> outputBuffer;

[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint old;
    InterlockedAdd(outputBuffer[0], 1u, old);
}

// Final value must be exactly 64: all 64 increments serialized in total order.
// CHECK: 64
