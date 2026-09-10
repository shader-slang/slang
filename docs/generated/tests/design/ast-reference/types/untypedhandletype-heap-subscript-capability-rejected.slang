//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#untypedresourcehandletype-and-untypedsamplerhandletype
//META: doc_section_digest=23856356bace576e711fa2b6686659e33aa5c5f0a6c6c1955ee17350d49ad6ca
//META: purpose=The descriptor-heap subscript is capability-gated, so writing ResourceDescriptorHeap[i] for a target outside the gated set is diagnosed, and the gate repeated on the subscript makes the note land at the indexing site rather than inside the core module.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the capability-gate claim. Metal is outside the
// target set the heap subscript requires, so the same source that emits on
// HLSL / GLSL / SPIR-V / WGSL is rejected here: CHECK pins the
// unavailable-features error code, and the caret annotation pins the note to
// the `[` of the user's own subscript. That second observation is the point of
// repeating the gate on the subscript — without it the note would point into
// the core module, where the user has nothing to fix.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;
uniform uint idx;

[numthreads(1, 1, 1)]
void computeMain()
{
    //CHECK: E36107
    Texture2D t = ResourceDescriptorHeap[idx];
    //CHECK:                            ^ note
    outputBuffer[0] = t.Load(int3(0, 0, 0)).x;
}
