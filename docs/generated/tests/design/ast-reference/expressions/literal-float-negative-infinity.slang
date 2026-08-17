//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
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
