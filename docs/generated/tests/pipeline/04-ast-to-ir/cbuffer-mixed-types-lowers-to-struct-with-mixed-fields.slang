//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=9ce4d053661212c26b75e2e5610820fbcc296703c400512972d2afca16087b34
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
