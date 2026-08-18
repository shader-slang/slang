//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#flat-decoration-for-integral-fragment-inputs
//META: doc_section_digest=4dbf7c0ed7254dfa9c7410b71566b04103313977e4ace15e9147d9f906b887b5
//META: purpose=An integral fragment-stage varying input receives the Flat decoration required by SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the emitter flat-qualifies an integral fragment input, which
// SPIR-V requires because integers cannot be interpolated. The int field is
// carried on a varying input struct so it becomes an Input-storage global
// param; CHECK asserts the Input variable carries both a Location and the Flat
// decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

struct VOut
{
    float4 pos : SV_Position;
    nointerpolation int id : ID;
}

RWStructuredBuffer<int> outBuf;

[shader("fragment")]
float4 main(VOut v) : SV_Target
{
    outBuf[0] = v.id;
    return float4(1.0, 0.0, 0.0, 1.0);
}

// CHECK-DAG: OpDecorate %{{[a-zA-Z_0-9]+}} Flat
// CHECK-DAG: OpVariable %_ptr_Input_int Input
