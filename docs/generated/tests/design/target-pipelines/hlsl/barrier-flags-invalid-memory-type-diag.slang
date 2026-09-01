//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#validatebarrierflagsforhlsl
//META: doc_section_digest=eee10a133fed78bc4a828626e05d33cb9d0ac877ce7842e5c39404afb4fb107d
//META: purpose=Negative: a barrier memory-type flag set with no HLSL named-constant spelling is reported as E31117 by the HLSL/D3D barrier-flag validation pass, so compilation stops before emit.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the front-half of the barrier-flag split: validation diagnoses a
// flag set the emit-side name table cannot spell, instead of letting the
// emitter hit an assertion. The test passes `0x80` — a bit outside the four
// documented memory-type bits — through a cast, so the value is a valid
// compile-time constant but not a valid flag set. The caret-anchored CHECKs
// assert the error fires on that call under the code the doc pins for the
// memory-type arm — E31117, as against E31116 for the semantic arm — and that
// the attached note enumerates the bits that *are* spellable, which shows the
// diagnostic (not a silent degradation) is what terminates the compile.
//
// The code rather than the message text is asserted on the first caret because
// a caret annotation consumes one diagnostic: pinning the code there leaves the
// second caret free for the note, and the code is the identity that survives a
// message rewording.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

import experimental.workgraph;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main()
{
    Barrier((BarrierMemoryTypeFlags)0x80, BarrierSemanticFlags.GROUP_SYNC);
//CHECK:   ^ E31117
//CHECK:   ^ unrecognized BarrierMemoryTypeFlags value '0x80'; expected a combination of UAV_MEMORY (0x1), GROUP_SHARED_MEMORY (0x2), NODE_INPUT_MEMORY (0x4), NODE_OUTPUT_MEMORY (0x8), or ALL_MEMORY (0xf)
}
