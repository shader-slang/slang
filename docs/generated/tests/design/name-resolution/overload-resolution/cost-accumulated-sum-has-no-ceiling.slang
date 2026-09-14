//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
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
