//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=`Optional<vector<half, 3>>` composes through lowering as `Optional(Vec(Half, 3 : Int))` — generic specialization of `Optional` over a narrow-precision vector element is deferred, but the storage IR type is observable.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: type-composition axis. A nested type
// (Optional of vector of half) round-trips through lowering and
// appears as a single `Optional(Vec(Half, 3 : Int))` IR type
// constant, exercising the doc's "VarDecl(local) -> IRVar" row
// alongside `BuiltinOperatorExpr/MemberExpr` ones on a non-trivial type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform vector<half, 3> v;
uniform int sel;
uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<vector<half, 3>> opt;
    if (sel > 0) { opt = v; }
    if (opt.hasValue) {
        buf[0] = opt.value.x;
    } else {
        buf[0] = half(0);
    }
}

// CHECK-LABEL: func %main
// CHECK: Optional(Vec(Half, 3 : Int))
