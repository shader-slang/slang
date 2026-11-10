//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#inlining-and-optimization
//META: doc_section_digest=be3dd448892d8840c35684e6b814bbb4a28a297daf99996e234ecacad1c060e9
//META: purpose=A function decorated with __unsafe_force_inline_early carries an unsafeForceInlineEarly decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `unsafeForceInlineEarly` row, the marker that requests
// inlining immediately after code generation rather than during the normal
// IR passes. The CHECK pins the bare decoration; the dump used here is the
// initial lowering snapshot, which is taken before that early inliner has
// had a chance to consume the marker.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[__unsafeForceInlineEarly]
int earlyInlined(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = earlyInlined(int(tid.x));
}

// CHECK: [unsafeForceInlineEarly]
