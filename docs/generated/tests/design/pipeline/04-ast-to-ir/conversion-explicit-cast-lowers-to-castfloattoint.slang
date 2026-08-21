//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
