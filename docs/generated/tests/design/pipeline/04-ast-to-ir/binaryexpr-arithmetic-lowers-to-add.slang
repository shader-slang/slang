//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A builtin arithmetic site (`a + b`, a checker fast-path BuiltinOperatorExpr) lowers to a single pure-value IR opcode (`add`) — the doc's "single pure value inst (IRAdd, IRMul, ...)" row.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `a + b` with two `uniform` operands lowers to `add(%a, %b)`.
// Non-constant operands are required because constant folding
// collapses literal arithmetic before lowering can dump it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + b;
}

// CHECK-LABEL: func %main
// CHECK: add(%{{.*}}, %{{.*}})
