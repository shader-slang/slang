//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=647327ee76a1bc33d8bcf7d2abc19a2f16ee4e63d10bad4d1d5d48c120029d9d
//META: purpose=A [branch] attribute on a conditional lowers to a branch decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `branch` row's operand shape: the decoration is nullary, so
// the dump prints a bare `[branch]` line on the conditional it annotates.
// The condition reads the dispatch thread id and the selected value escapes
// through a buffer write, so neither folding nor DCE can remove the
// conditional before the CHECK sees its decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = 0;
    [branch]
    if (tid.x == 0u) v = 10;
    else v = 20;
    buf[0] = v;
}

// CHECK: [branch]
