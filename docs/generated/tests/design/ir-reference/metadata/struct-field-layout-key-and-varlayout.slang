//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=0e7a8ec2a490ce368dde09c10d45863e67fc1e491cc433074fe4f7bbf890e750
//META: purpose=A structFieldLayout attribute stores the field key in operand 0 and the field's varLayout in operand 1.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two operands of a field-layout attribute. The constant buffer
// has a single field named `color`, so the attribute's first operand prints as
// that field key; the CHECK captures the second operand and resolves it to a
// `varLayout` definition, which is what proves the key/layout pairing rather
// than two layouts or two keys.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB
{
    float4 color;
}

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = color.x;
}

// CHECK: = structFieldLayout(%color, %[[VL:[0-9]+]])
// CHECK: %[[VL]]{{.*}}= varLayout(
