//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=4b61bbe5577fdd277138f6c76e47bb415b4e0d9c6365f3fd1961205026f01739
//META: purpose=Boundary - the [MaxIters(N)] attribute produces a loopMaxIters decoration carrying the upper-bound integer N on the loop.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `loopMaxIters` records the user-supplied iteration bound as
// an integer operand. The loop's trip count is thread-id dependent so it
// cannot be resolved at compile time, which is exactly the case the bound
// exists for; the CHECK pins `[loopMaxIters(10 : Int)]`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [MaxIters(10)]
    for (int i = 0; i < int(tid.x); i++) s += i;
    buf[0] = s;
}

// CHECK: [loopMaxIters(10 : Int)]
