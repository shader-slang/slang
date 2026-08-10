//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=667410b964ed99fbf0c70015e2ab77e7656e9f8728c17d66aa0a1cad3ef6f6b5
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
