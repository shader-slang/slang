//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A LiteralExpr lowers to a constant IR instruction (IRIntLit / IRFloatLit) visible as the literal value inline.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists "LiteralExpr → constant inst (IRIntLit, IRFloatLit,
// ...)". In the dump these constants appear inline as operands of
// the operator that consumes them, e.g. the `1` in `add(%a, 1 : Int)`
// or the `3.5` in `mul(%a, 3.5 : Float)`. The `: Int` / `: Float`
// suffix is the constant inst's type annotation.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform float b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = a + 1;
    float f = b * 3.5;
    buf[0] = float(i) + f;
}

// CHECK-LABEL: func %main
// CHECK: add(%{{.*}}, 1 : Int)
// CHECK: mul(%{{.*}}, 3.5 : Float)
