//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A BlockStmt containing a single statement (no control flow) lowers to a single entry IRBlock; lowering does not introduce extra blocks just because of the brace nesting.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: BlockStmt lower edge — a brace-only nested block with
// one statement. Doc says "BlockStmt → A sequence of basic blocks";
// the lower bound of "sequence" is exactly one block when there is
// no branching. The function body has a single block ending in
// `return_val`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    { buf[0] = helper(a); }
}

// CHECK-LABEL: func %main
// CHECK: block %{{.*}}
// CHECK: call %helper
// CHECK: return_val(void_constant)
