//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=Boundary probe of floating-point suffix classification: an `h` suffix makes the literal a Half, so a value above the half maximum decodes to INFINITY rather than staying finite.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the value-extraction helper picks the literal's type
// from its suffix and then rounds into that type's range, not into
// double's. 70000 is comfortably finite as a float or a double but is
// above the largest finite half (65504), so classifying the `h`
// suffix as Half is the only way the decoded value becomes infinite.
// Infinity is the one positive value that halving leaves unchanged, so
// the predicate prints 1 only for a true INFINITY; a value merely
// clamped to the half maximum would print 0.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float h = 70000.0h;
    // CHECK: halfinf=1
    printf("halfinf=%d\n", (h > 0.0 && h * 0.5 == h) ? 1 : 0);
}
