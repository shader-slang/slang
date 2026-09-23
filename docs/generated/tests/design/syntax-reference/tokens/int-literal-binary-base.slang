//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=c2efc22fd6b393971554625ec6e5d1342ed8b02f96b9afdf61ab48806b3d700e
//META: purpose=Boundary - a binary integer literal (`0b1111`) lexes as IntegerLiteral and decodes to its base-2 value.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: a leading `0b` followed by binary digits is part
// of the IntegerLiteral token; `0b1111` should decode to decimal 15.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 0b1111;
    // CHECK: v=15
    printf("v=%d\n", v);
}
