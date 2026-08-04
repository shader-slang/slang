//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=Verifies that an arithmetic underflow to -inf compares less than the smallest finite (negated MAX) float literal.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// -inf boundary. Multiplying the negated largest finite by 10 yields
// -inf; the result must order strictly below the most negative finite.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float big = 3.4e38f;
    float ninfv = -big * 10.0f;
    // CHECK: lt-min=1
    printf("lt-min=%d\n", int(ninfv < -big));
}
