//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#varlayout-and-entrypointlayout
//META: doc_section_digest=0ce1b4071a5a571cfc1945ea83ae995cc4721673793ce1d486650e97ce5658bb
//META: purpose=An EntryPointLayout has exactly two fixed operands and no attribute tail: the parameters layout and the result layout, both varLayouts.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the entry-point layout's distinct shape. The CHECK pins the closing
// parenthesis right after the second operand, so a third operand (an attribute
// tail) would fail the match, and each captured operand is then shown to be
// defined by a `varLayout` -- the documented parameters/result pair.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = EntryPointLayout(%[[PARAMS:[0-9]+]], %[[RESULT:[0-9]+]])
// CHECK: %[[RESULT]]{{.*}}= varLayout(
// CHECK: %[[PARAMS]]{{.*}}= varLayout(
