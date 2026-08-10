//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=43c0f4ec58392bc558dea4e760d5be974187198677d818ffcac504b279a89de3
//META: purpose=An offset attribute stores kind and offset only, omitting the register-space operand when the space is zero.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the optional space operand. Nothing in this
// module asks for a non-default register space, so every emitted `offset` must
// take the two-operand form; the CHECK pins the closing parenthesis
// immediately after the offset value and the CHECK-NOT asserts no
// three-operand form appears anywhere in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = offset({{[0-9]+}} : Int, {{[0-9]+}} : Int)
// CHECK-NOT: = offset({{[0-9]+}} : Int, {{[0-9]+}} : Int, {{[0-9]+}} : Int)
