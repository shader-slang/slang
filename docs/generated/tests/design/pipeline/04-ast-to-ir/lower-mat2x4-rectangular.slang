//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A non-square `float2x4` matrix lowers with IR type `Mat(Float, 2 : Int, 4 : Int, 0 : Int)`; the row/column operands appear in source order.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: matrix-shape axis — rectangular (rows != columns).
// Verifies that the IR's Mat(...) row-then-column operand order
// matches the source's R x C declaration order.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float2x4 m;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = m[0][0];
}

// CHECK: [nameHint("m")]
// CHECK: let {{.*}}m{{.*}}: Mat(Float, 2 : Int, 4 : Int, 0 : Int){{.*}}= global_param
