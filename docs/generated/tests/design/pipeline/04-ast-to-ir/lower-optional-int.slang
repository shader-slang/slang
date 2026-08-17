//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
