//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A ReturnStmt nested inside a ForStmt lowers to a `return_val(...)` terminator in a block reached from the loop body, alongside the `loop(...)` structured-branch terminator — the doc's "ReturnStmt -> IRReturn" claim is unaffected by enclosing control flow.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: return-stmt placement axis — early return inside a
// loop. The IRFunc body contains both a `loop(...)` (for the for-
// loop) and a `return_val(...)` (for the in-loop return), confirming
// that lowering encodes the return as a terminator irrespective of
// nesting.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform int target;
uniform RWStructuredBuffer<int> buf;

int findFirst(int t, int upTo)
{
    for (int i = 0; i < upTo; i++) {
        if (i == t) return i;
    }
    return -1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = findFirst(target, n);
}

// CHECK-LABEL: func %findFirst
// CHECK: loop(
// CHECK: return_val(%{{.*}})
// CHECK: return_val(-1 : Int)
