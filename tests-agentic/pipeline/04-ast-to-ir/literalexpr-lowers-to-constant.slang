//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
