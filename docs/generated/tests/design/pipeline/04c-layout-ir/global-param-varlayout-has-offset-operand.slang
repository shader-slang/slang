//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-global-parameter-steps
//META: doc_section_digest=75ec39da5e327b247f3579e584f2bfdf8a7756d5bc9f8880dffb38a09c79480d
//META: purpose=Lowering a global parameter's variable layout produces a varLayout instruction that carries an offset operand encoding the per-target placement.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the per-global-parameter lowering step emits a varLayout
// instruction whose operands record where the parameter was placed. CHECK
// looks for a `varLayout(` instruction followed by an `offset(` operand
// definition in the merged layout block; the offset operand is what
// encodes the binding/byte placement the parameter-binding pass decided.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CHECK: varLayout(
// CHECK: offset(
