//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Boundary - a CharLiteral whose content is an escape sequence (`'\\'`, `'\n'`) lexes as one token and decodes to the escaped code point.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: an escape sequence inside single quotes is a
// single CharLiteral. `'\\'` is backslash (ASCII 92); `'\n'` is
// newline (ASCII 10).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: bs=92
    printf("bs=%d\n", int('\\'));
    // CHECK: nl=10
    printf("nl=%d\n", int('\n'));
}
