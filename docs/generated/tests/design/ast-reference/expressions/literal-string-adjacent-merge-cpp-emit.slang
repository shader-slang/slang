//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d9d7b18fdd616b76c0820daa714d668ab15618d45e2e3e82fc9738024229a9cb
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
