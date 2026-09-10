//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#angle-bracket-annotations
//META: doc_section_digest=7ce4d4ac217c6c67d9cc75b8b918d498cbe8d315518ddcbd63412016bbc998ce
//META: purpose=The semantic-level annotation skip is unconditional, so a `< ... >` clause written after a semantic such as `: SV_Position` is advanced past with no flag and nothing inside it reaches the AST.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second, independent angle-bracket skip and the fact that it
// needs no opt-in: this file carries no `-enable-effect-annotations`, and the
// clause after `SV_Position` is still discarded. Because a semantic has already
// been read at that point there is no competing generic reading to resolve, so
// unlike the declarator-level skip there is no disambiguation and no flag. The
// CHECK pins the field emitting with its bare semantic, and the CHECK-NOT lines
// assert the clause's contents left no trace.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage vertex

struct VOut
{
    float4 pos : SV_Position <string ann = "hello"; int k = 2;>;
    float2 uv  : TEXCOORD0;
};

//CHECK: float4 pos_{{[0-9]+}} : SV_Position;
//CHECK-NOT: ann
//CHECK-NOT: hello

VOut main(uint id : SV_VertexID)
{
    VOut o;
    o.pos = float4(float(id), 0, 0, 1);
    o.uv = float2(0, 0);
    return o;
}
