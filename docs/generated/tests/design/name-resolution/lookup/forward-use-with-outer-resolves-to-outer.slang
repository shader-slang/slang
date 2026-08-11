//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#block-local-shadowing
//META: doc_section_digest=fff04d2b441a5ad2e4ef387152ddc76216942a0668864eb5cdfb1c9b98e7e532
//META: purpose=A local is hidden from lookup until its own declaration statement is reached, so a reference earlier in the block binds to a same-named outer decl instead.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that block-local hiding is a lookup filter, not an outright
// error: the inner `q` is skipped at the earlier use site, so the walk
// continues outward and finds the file-scope `q`. CHECK asserts the
// earlier statement saw 1000 while the later print of the same
// spelling sees the local 7, which separates hiding from shadowing.

//TEST:INTERPRET(filecheck=CHECK):

static const int q = 1000;

void main()
{
    int y = q + 1;
    int q = 7;
    // CHECK: y=1001 inner=7
    printf("y=%d inner=%d\n", y, q);
}
