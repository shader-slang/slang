//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T20:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
