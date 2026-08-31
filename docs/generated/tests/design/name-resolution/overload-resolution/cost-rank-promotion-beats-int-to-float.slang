//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_RankPromotion (150) is cheaper than kConversionCost_IntegerToFloatConversion (400), so widening int to int64_t beats converting it to float.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ordering of two same-kind-vs-different-kind conversions: a
// lossless widening within the integer family is ranked far below crossing into
// the floating-point family. CHECK asserts the int64_t overload fired.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int64_t x) { return 1; }
int pick(float x)   { return 2; }

void main()
{
    int a = 5;
    int tag = pick(a);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
