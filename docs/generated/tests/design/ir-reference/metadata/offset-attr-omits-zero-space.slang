//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=c9513e1455ac8d0e90436e3db8230754dc3be3e0a40bcd7e4c788a745e88d3e7
//META: purpose=An offset attribute stores kind and offset only, omitting the register-space operand when the space is zero.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the optional space operand. Nothing in this
// module asks for a non-default register space, so every emitted `offset` must
// take the two-operand form; the CHECK pins the closing parenthesis
// immediately after the offset value and the CHECK-NOT asserts no
// three-operand form appears anywhere in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = offset({{[0-9]+}} : Int, {{[0-9]+}} : Int)
// CHECK-NOT: = offset({{[0-9]+}} : Int, {{[0-9]+}} : Int, {{[0-9]+}} : Int)
