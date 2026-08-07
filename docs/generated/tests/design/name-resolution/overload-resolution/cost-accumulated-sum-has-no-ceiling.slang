//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=d77239a753c95159f4db29b226999df363503e43e2ce42ec3d20143496696e2b
//META: purpose=Probing applies no ceiling to conversionCostSum, so a candidate whose per-argument costs sum past kConversionCost_GeneralConversion (900) is still applicable.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the implicit-conversion ceiling is not applied to the ranking
// sum. Three int64_t arguments narrowing to int8_t each cost 450, summing to
// 1350, well past the 900 threshold; the only candidate is still selected and
// invoked. CHECK asserts its tag, which would be unreachable if the sum were
// capped.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int8_t a, int8_t b, int8_t c) { return 1; }

void main()
{
    int64_t x = 1;
    int tag = pick(x, x, x);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
