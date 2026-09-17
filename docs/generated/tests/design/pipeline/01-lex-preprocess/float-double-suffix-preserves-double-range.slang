//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=60402c8d086f96269e6fe5211e0043b74eb989c5d25aea5312e6a4ba5d5f8bf4
//META: purpose=Boundary probe of floating-point suffix classification: an `lf` suffix makes the literal a Double so 1e300 stays finite, while the same digits with an empty suffix are a Float and overflow to INFINITY.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Double and Float arms of suffix classification against
// each other on the same significand. 1e300 is representable as a
// double but far above the float maximum, so the two decoded values
// must differ purely because of the suffix. The finiteness predicate
// (halving changes a finite value, but leaves INFINITY unchanged)
// prints 1 in both cases only if the `lf` literal really stayed
// finite and the unsuffixed literal really overflowed.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    double withSuffix = 1e300lf;
    // CHECK: double-finite=1
    printf("double-finite=%d\n", (withSuffix > 0.0 && withSuffix * 0.5 != withSuffix) ? 1 : 0);

    float noSuffix = 1e300;
    // CHECK: float-inf=1
    printf("float-inf=%d\n", (noSuffix > 0.0 && noSuffix * 0.5 == noSuffix) ? 1 : 0);
}
