//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=28a5007e7990e447eb6cc6141908413f26bb0721bb1b7dcb8e454fd24d38dbb8
//META: purpose=Stress - multiple `/*` sequences inside a block comment do not stack; the comment still ends at the first `*/` regardless of how many fake openers precede it.
//META: intent=stress
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: four fake nested-comment openers in a row, then a
// single closer. The doc says nesting is not supported, so the
// BlockComment must close at the FIRST `*/` and `static const int x`
// must still be a real declaration.

//TEST:INTERPRET(filecheck=CHECK):

/* /* /* /* still one block comment */ static const int x = 13;

void main()
{
    // CHECK: x=13
    printf("x=%d\n", x);
}
