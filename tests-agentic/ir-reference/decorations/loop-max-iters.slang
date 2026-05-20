//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=d7cf9915ec0da5de567edb118ec898dd57bb3e7b5414db216254906018825537
//META: purpose=Boundary - the [MaxIters(N)] attribute produces a loopMaxIters decoration carrying the upper-bound integer N on the loop.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc lists `loopMaxIters` with operands
// "(variadic, min=1)" and AST origin `[loop_count(...)]`. The
// observed spelling that produces this decoration is `[MaxIters(N)]`,
// flowing the integer through into a single IRIntLit operand.

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
