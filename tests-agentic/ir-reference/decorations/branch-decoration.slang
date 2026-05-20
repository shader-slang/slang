//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=d7cf9915ec0da5de567edb118ec898dd57bb3e7b5414db216254906018825537
//META: purpose=A [branch] attribute on a conditional lowers to a branch decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `branch` ("Hints the backend to emit a branching
// select") with no operands. The IR dump emits a bare `[branch]`
// line on the decorated inst.

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
