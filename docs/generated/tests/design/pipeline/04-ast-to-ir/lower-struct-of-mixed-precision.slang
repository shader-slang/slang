//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A struct mixing narrow integer, half, float, double, and uint16 fields lowers so each `field(...)` row carries its element's lowered IR type verbatim.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: combined storage axes — narrow integer, half,
// float, double, and uint16 fields side by side. Verifies that
// the per-field IR type carries through the StructDecl row
// without any homogenization to a common precision.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Mixed {
    int8_t i8;
    half h;
    float f;
    double d;
    uint16_t u16;
}

uniform Mixed m;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float(m.i8) + float(m.h) + m.f + float(m.d) + float(m.u16);
}

// CHECK-LABEL: struct %Mixed
// CHECK: field({{.*}}, Int8)
// CHECK: field({{.*}}, Half)
// CHECK: field({{.*}}, Float)
// CHECK: field({{.*}}, Double)
// CHECK: field({{.*}}, UInt16)
