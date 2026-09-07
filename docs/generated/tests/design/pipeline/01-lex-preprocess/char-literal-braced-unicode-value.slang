//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=Boundary probe of the braced `\u{...}` escape: a code point above the BMP (`'\u{1F600}'`) decodes to 128512 as a 32-bit value.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the braced Unicode escape form with a supplementary-plane
// code point: U+1F600 = 128512 does not fit in 16 bits, so the
// printed value is only correct if the braced form really carries
// up to 32 bits through the char value-extraction helper.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: braced=128512
    printf("braced=%d\n", int('\u{1F600}'));
}
