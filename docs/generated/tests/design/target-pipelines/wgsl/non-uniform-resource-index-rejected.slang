//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#floatnonuniformresourceindex
//META: doc_section_digest=0164b19f2e0058532e69cb944ccea01bf6885f2346f7b081c69f5d94bf437802
//META: purpose=NonUniformResourceIndex is rejected for a WGSL compute entry point with E36107 so floatNonUniformResourceIndex's textual-mode wrapper never reaches WGSL emit
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The cited section reasons about what the WGSL emitter does with a
// NonUniformResourceIndex(...) wrapper that survives to emit. This test
// records the boundary that sits in front of that reasoning: the core
// module's capability set for the intrinsic excludes WGSL, so the entry
// point is rejected at type-check time and the wrapper never enters the
// WGSL pipeline. The caret annotation pins the entry-point-level report and
// the E36107 code; the two follow-on `see using of` / `see definition of`
// notes are intentionally unannotated, hence non-exhaustive.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
StructuredBuffer<float> bufs[4];

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'wgsl' compilation target.
*/
// CHECK: E36107
{
    outBuf[tid.x] = bufs[NonUniformResourceIndex(tid.x)][0];
}
