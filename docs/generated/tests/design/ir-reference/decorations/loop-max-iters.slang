//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=dc9cebc6b1ccfabf37d9ddb054319d9e5160d3b17890ab2828db7671ae5490a0
//META: purpose=Boundary - the [MaxIters(N)] attribute produces a loopMaxIters decoration carrying the upper-bound integer N on the loop.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `loopMaxIters` records the user-supplied iteration bound as
// an integer operand. The loop's trip count is thread-id dependent so it
// cannot be resolved at compile time, which is exactly the case the bound
// exists for; the CHECK pins `[loopMaxIters(10 : Int)]`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

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
