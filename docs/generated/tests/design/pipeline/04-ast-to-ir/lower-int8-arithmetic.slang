//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A `BuiltinOperatorExpr` on `int16_t` operands lowers to `add` whose result type carries through the narrow `Int16` IR type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: pure-value-inst row, narrow-integer side. The doc's
// "BuiltinOperatorExpr -> IRAdd/..." claim must hold when both operands
// are narrow integer types — the resulting `add` carries `Int16`
// in its operand types.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int16_t a;
uniform int16_t b;
uniform RWStructuredBuffer<int16_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int16_t(a + b);
}

// CHECK-LABEL: func %main
// CHECK: Int16{{.*}}= add(%{{.*}}, %{{.*}})
