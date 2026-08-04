//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=19eb09c072c556dd0799b5002c2f1b24781da93f2cebc122ca027062c24e0b84
//META: purpose=An explicit constructor-style cast `int(x)` between primitive numeric types lowers to a dedicated conversion IR opcode (`castFloatToInt` / `castIntToFloat`), produced from a `TypeCastExpr` AST node.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's mapping row covers the checker fast-path
// "BuiltinOperatorExpr → a single pure value inst" but explicit
// conversions are also pure
// value insts in the same family. `int(a)` on a `float a` lowers to
// `castFloatToInt(%a)`; `float(i)` lowers to `castIntToFloat(%i)`.
// Use constructor-style casts (per `_common.md`), not C-style.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int casted = int(a);
    buf[0] = float(casted);
}

// CHECK-LABEL: func %main
// CHECK: castFloatToInt(%{{.*}})
// CHECK: castIntToFloat(%{{.*}})
