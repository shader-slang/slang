//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=fefaa116238af2b286c6cb73dc3224a4704918ed8d0cdbaa5e118e7c7b474c44
//META: purpose=Boundary - a `: packoffset(c2.y)` modifier lowers to a packoffset decoration with (registerOffset, componentOffset) = (2, 1).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `packoffset` splits the HLSL `cN.component` spelling into
// two separate integer operands. Two fields are declared, one at `c0`
// (both operands zero) and one at `c2.y`, so the CHECK-DAG lines pin both
// the all-zero corner and a case where the component letter must map to a
// non-zero index.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer MyCB
{
    float4 a : packoffset(c0);
    float4 b : packoffset(c2.y);
};

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a.x + b.y + float(tid.x);
}

// CHECK-DAG: [packoffset(0 : Int, 0 : Int)]
// CHECK-DAG: [packoffset(2 : Int, 1 : Int)]
