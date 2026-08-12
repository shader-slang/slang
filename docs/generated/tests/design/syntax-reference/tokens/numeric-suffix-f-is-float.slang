//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=The `f` suffix on a floating-point literal types the token as `float` (single precision), distinguishing it from the unsuffixed form.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `3.5f` is typed as `float`. Sent through an overload
// set that distinguishes `float` from `double` — the suffix decides
// which overload is picked.

//TEST:INTERPRET(filecheck=CHECK):

int kind(float v) { return 1; }
int kind(double v) { return 2; }

void main()
{
    // CHECK: kind=1
    printf("kind=%d\n", kind(3.5f));
}
