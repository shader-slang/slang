//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#token-flags
//META: doc_section_digest=8fe6488c7be4132d8fb4865bd94f376537bdb8628cd9f0ce009cc94d674aea9b
//META: purpose=A folded `\\`-newline emits no `NewLine` token, so the token after it is not marked as starting a logical line and a macro body can span two physical lines.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the interaction the `AtStartOfLine` flag exists for: the
// preprocessor ends a directive at the first token that starts a logical line,
// and a folded continuation must not create one. The macro body lives on the
// physical line after the backslash; if that line had been flagged as a line
// start, the body would have been dropped from the definition and the
// expansion would not compute 42.

//TEST:INTERPRET(filecheck=CHECK):

#define BUMP(x) \
    ((x) + 1)

void main()
{
    // CHECK: bump=42
    printf("bump=%d\n", BUMP(41));
}
