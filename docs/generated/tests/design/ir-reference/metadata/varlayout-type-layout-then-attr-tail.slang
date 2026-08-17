//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#varlayout-and-entrypointlayout
//META: doc_section_digest=0ce1b4071a5a571cfc1945ea83ae995cc4721673793ce1d486650e97ce5658bb
//META: purpose=A varLayout has one fixed operand, the type layout, followed by a variable-length tail of attributes.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand shape of a per-variable layout. The buffer parameter's
// `varLayout` is captured with two operands; following the first must reach a
// concrete type-layout opcode and following the second must reach an `offset`
// attribute. That combination is only consistent with "operand 0 is the type
// layout, everything after it is the attribute tail".

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: %{{[0-9]+}}{{.*}}= varLayout(%[[TL:[0-9]+]], %[[OFF:[0-9]+]])
// CHECK: %[[OFF]]{{.*}}= offset(
// CHECK: %[[TL]]{{.*}}= structuredBufferTypeLayout(
