//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=60402c8d086f96269e6fe5211e0043b74eb989c5d25aea5312e6a4ba5d5f8bf4
//META: purpose=Boundary probe of floating-point value extraction: a literal whose magnitude is above the maximum for its literal type parses as INFINITY.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the overflow arm of floating-point literal decoding:
// 1e400 is beyond every finite value, so the extracted value must
// be positive infinity. Infinity is the only positive value that
// halving leaves unchanged, so the predicate below prints 1 only if
// the literal really decoded to INFINITY (a clamp to the largest
// finite value would print 0).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float f = 1e400;
    // CHECK: inf=1
    printf("inf=%d\n", (f > 0.0 && f * 0.5 == f) ? 1 : 0);
}
