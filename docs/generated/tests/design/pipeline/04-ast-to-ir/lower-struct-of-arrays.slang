//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T20:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A struct whose fields are fixed-size arrays lowers so each `field(...)` row carries an `Array(...)` IR type — composition of StructDecl + Array rows.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: composition axis — struct-of-arrays. The struct's
// `field(...)` rows each carry the lowered array IR type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Soa {
    int xs[4];
    float ys[4];
}

uniform Soa soa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float(soa.xs[0]) + soa.ys[0];
}

// CHECK-LABEL: struct %Soa
// CHECK: field({{.*}}, Array(Int, 4 : Int))
// CHECK: field({{.*}}, Array(Float, 4 : Int))
