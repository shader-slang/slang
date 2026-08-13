//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Boundary - the integer literal `0` lexes as IntegerLiteral with int type at the lower edge of the int range.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the smallest non-negative integer literal `0` is
// still an `IntegerLiteral` token and types as `int` in the overload
// set below.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v) { return 1; }
int kind(uint v) { return 2; }

void main()
{
    // CHECK: kind=1
    printf("kind=%d\n", kind(0));
}
