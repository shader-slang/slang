//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_InRangeIntLitSignedToUnsignedConversion (32) is cheaper than kConversionCost_IntegerToFloatConversion (400), so an in-range integer literal selects the uint overload.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the in-range-literal cost level, which only applies to a literal (a
// non-literal int would instead pay the 250-level signed-to-unsigned cost).
// Both overloads are viable for the literal 3; CHECK asserts the uint one won.

//TEST:INTERPRET(filecheck=CHECK):

int pick(uint x)  { return 2; }
int pick(float x) { return 3; }

void main()
{
    int tag = pick(3);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
