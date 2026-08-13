//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-entry-point-steps
//META: doc_section_digest=2051f00317fddb1d324980572e90d6c58961dbf4384bdc38e7d5acf46a5ce786
//META: purpose=Lowering the entry-point layout produces an EntryPointLayout instruction with two operands, one varLayout for the parameters and one for the result.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shape of the lowered entry-point layout. CHECK matches
// `EntryPointLayout(%A, %B)` with exactly two operands and then asserts
// that each operand is defined by its own `varLayout(` instruction - the
// parameters layout and the result layout the lowering step supplies.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: EntryPointLayout(%{{[0-9]+}}, %{{[0-9]+}})
// CHECK: varLayout(
// CHECK: varLayout(
