//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=d246c5b26aea56aab3063bb078408e447e5266c12518eab4e032ae8f4ee5812b
//META: purpose=A cbuffer mixing scalar, vector, and matrix fields lowers to a `ConstantBuffer<...>` payload struct whose `field(...)` rows carry each member's IR type verbatim (Int, Vec, Mat).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: cbuffer mixed-types axis. Verifies that the
// per-field IR-type representation is preserved through lowering —
// scalar `Int`, `Vec(Float, 3)`, and `Mat(Float, 4, 4, 0)` appear
// as the three `field(...)` row types in declaration order.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

cbuffer CB {
    int i;
    float3 v;
    float4x4 m;
}

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float(i) + v.x + m[0][0];
}

// CHECK-LABEL: struct %SLANGx5FParameterGroupx5FCB
// CHECK: field(%i, Int)
// CHECK: field(%v, Vec(Float, 3 : Int))
// CHECK: field(%m, Mat(Float, 4 : Int, 4 : Int, 0 : Int))
