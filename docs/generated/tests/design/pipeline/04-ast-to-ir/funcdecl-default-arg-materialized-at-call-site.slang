//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A FuncDecl with a default-valued parameter lowers normally; the omitted argument at the call site is materialized as a literal operand in the `call` instruction.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: function-parameter default-value edge. The `d = 5`
// default is not visible on the callee's IRFunc shape (which still
// declares both params); instead lowering produces a `call
// %helper(%a, 5 : Int)` at the call site that fills in the default
// as a literal IntLit operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int v, int d = 5) { return v + d; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(a);
}

// CHECK-LABEL: func %helper
// CHECK: param %v
// CHECK: param %d
// CHECK-LABEL: func %main
// CHECK: call %helper(%a, 5 : Int)
