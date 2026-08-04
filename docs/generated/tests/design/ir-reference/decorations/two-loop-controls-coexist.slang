//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=d7cf9915ec0da5de567edb118ec898dd57bb3e7b5414db216254906018825537
//META: purpose=Two for-loops decorated with different loop-control attributes ([unroll] vs [loop]) each get their own loopControl decoration, demonstrating that two such decorations coexist in the same function.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `loopControl` as having a per-loop `modeOperand:
// IRConstant`. Two adjacent for-loops with distinct attributes
// should both surface their decorations; the dump records each
// at the appropriate loop site.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [unroll]
    for (int i = 0; i < 4; i++) s += int(tid.x) * i;
    [loop]
    for (int j = 0; j < 8; j++) s += int(tid.y) * j;
    buf[0] = s;
}

// CHECK-DAG: [loopControl(0 : Int)]
// CHECK-DAG: [loopControl(1 : Int)]
