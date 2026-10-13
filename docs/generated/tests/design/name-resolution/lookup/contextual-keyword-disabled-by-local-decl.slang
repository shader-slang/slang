//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#keyword-vs-identifier
//META: doc_section_digest=8b7942e8acf1087093f6159ded21d514eed4df7420306fe069948b19fb126f52
//META: purpose=A contextual keyword is only available when plain lookup of that identifier finds nothing, so a user local of the same spelling makes the parser treat the token as an ordinary identifier.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that keywords and user decls share one identifier namespace
// and lookup decides between them. `each` is a contextual keyword that
// introduces a pack-expansion element; with no local of that name the
// same text is a syntax error outside an `expand`. Declaring a local
// `each` makes lookup succeed, which disables the keyword. CHECK
// asserts 41 + 1, so the token was read as the variable.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int each = 41;
    int r = each + 1;
    // CHECK: r=42
    printf("r=%d\n", r);
}
