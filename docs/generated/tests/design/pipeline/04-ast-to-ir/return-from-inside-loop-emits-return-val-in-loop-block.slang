//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
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
