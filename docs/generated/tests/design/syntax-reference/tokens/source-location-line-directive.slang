//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#source-location
//META: doc_section_digest=e305e50fe49578bb01a84e7827ab633e6a337162c5659b6464350d5909536373
//META: purpose=A token's nominal source location honours `#line`, so `__LINE__` after `#line 100` reports 100 rather than the physical line.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `Nominal` interpretation of a token's `SourceLoc` is the one the
// preprocessor exposes. `__LINE__` is the only user-visible read of that
// decoded location, so printing it directly after a `#line` directive shows
// whether the directive was honoured; a physical-line ("Actual") reading would
// print this file's real line number instead.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: line=100
#line 100
    printf("line=%d\n", __LINE__);
}
