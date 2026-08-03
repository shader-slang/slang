//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=Boundary probe of floating-point value extraction: a literal whose magnitude overflows the type parses as INFINITY.
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
