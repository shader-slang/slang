//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
