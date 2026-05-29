//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
