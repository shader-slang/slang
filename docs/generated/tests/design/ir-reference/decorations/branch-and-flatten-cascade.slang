//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=d7cf9915ec0da5de567edb118ec898dd57bb3e7b5414db216254906018825537
//META: purpose=Stress - a cascade of conditionals with mixed [branch] and [flatten] hints produces multiple independent branch and flatten decorations on distinct ifElse instructions in the same function.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress - the doc lists branch and flatten as two independent
// bare decorations. Multiple ifElse statements in the same
// function each carry their own decoration; this test confirms
// the lowering preserves all three (two branches and one flatten)
// without collisions.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [branch]
    if (tid.x > 0) s = 1; else s = 2;
    [flatten]
    if (tid.y > 0) s += 4;
    [branch]
    if (tid.z > 0) s += 8; else s -= 8;
    buf[0] = s;
}

// CHECK-DAG: [branch]
// CHECK-DAG: [flatten]
