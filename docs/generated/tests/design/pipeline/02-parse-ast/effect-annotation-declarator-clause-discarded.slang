//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#angle-bracket-annotations
//META: doc_section_digest=7ce4d4ac217c6c67d9cc75b8b918d498cbe8d315518ddcbd63412016bbc998ce
//META: purpose=Under `-enable-effect-annotations` the declarator-level `< ... >` clause is discarded entirely, so nothing written inside the angle brackets reaches the AST and the annotated declaration binds normally.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the declarator-level annotation skip and, crucially, that it is a
// *discard* rather than a parse into something. `tex` carries a clause naming
// `uiName` and `uiOrder`; the CHECK-NOT lines assert neither name survives into
// the emitted HLSL, which they would if the clause had produced declarations or
// modifiers. The positive CHECK on the texture's own register binding is what
// separates "the clause was thrown away" from "the whole declaration was thrown
// away". The `;` inside the clause is what identifies it as an annotation
// rather than a generic argument list.

//TEST:SIMPLE(filecheck=CHECK):-enable-effect-annotations -target hlsl -entry main -stage compute

Texture2D tex <string uiName = "Tex"; int uiOrder = 3;>;
SamplerState samp;
RWStructuredBuffer<float> outputBuffer;

//CHECK: Texture2D{{.*}}tex_{{[0-9]+}} : register(t0)
//CHECK-NOT: uiName
//CHECK-NOT: uiOrder

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}
