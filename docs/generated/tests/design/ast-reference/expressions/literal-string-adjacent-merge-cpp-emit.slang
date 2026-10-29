//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=Adjacent string literals merge during expression parsing into a single StringLiteralExpr; the concatenated text appears as one literal in emitted C++.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that two adjacent string-literal tokens merge into one string
// value during expression parsing. The printf argument is written as two
// juxtaposed literals; CHECK matches the concatenation as one contiguous
// run of text in the emitted C++, which cannot match if the two pieces
// stayed separate quoted literals.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("agentic-merge-head-" "agentic-merge-tail");
    // CHECK: agentic-merge-head-agentic-merge-tail
}
