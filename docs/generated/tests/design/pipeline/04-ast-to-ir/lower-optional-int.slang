//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T20:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=`Optional<int>` composes through lowering as `Optional(Int)`; the nested-type axis on a scalar inner type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: nested type — `Optional` over a plain scalar type.
// Paired with `lower-optional-half3.slang`, this isolates the
// `Optional<T>` lowering from the half/vector axes.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int x;
uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<int> opt;
    if (sel > 0) { opt = x; }
    if (opt.hasValue) {
        buf[0] = opt.value;
    } else {
        buf[0] = 0;
    }
}

// CHECK-LABEL: func %main
// CHECK: Optional(Int)
