//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#expressions
//META: doc_section_digest=da8363e39914cfb7152b84e17c34f7e9f5e6ae76aad2de17997f38aecabfc651
//META: purpose=Ternary `?:` (level 13) is right-associative; `c1 ? a : c2 ? b : c` parses as `c1 ? a : (c2 ? b : c)`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies right-associativity of the ternary. With c1 false and
// c2 true, the chain returns `b` (5). Left-associative grouping
// `((c1 ? a : c2) ? b : c)` would also be ill-typed for non-bool
// `a`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int c1 = 0, c2 = 1;
    int r = c1 != 0 ? 1 : c2 != 0 ? 5 : 9;
    // CHECK: r=5
    printf("r=%d\n", r);
}
