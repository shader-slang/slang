//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=19eb09c072c556dd0799b5002c2f1b24781da93f2cebc122ca027062c24e0b84
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
