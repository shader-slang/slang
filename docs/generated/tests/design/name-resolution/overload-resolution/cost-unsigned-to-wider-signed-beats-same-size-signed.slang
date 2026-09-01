//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_UnsignedToSignedPromotion (200) is cheaper than kConversionCost_SameSizeUnsignedToSignedConversion (300), so a uint argument prefers int64_t over int.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two unsigned-to-signed levels against each other. A uint
// argument can widen losslessly into int64_t (200) or reinterpret into the
// same-size int (300); CHECK asserts the int64_t overload fired.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)     { return 1; }
int pick(int64_t x) { return 2; }

void main()
{
    uint a = 3;
    int tag = pick(a);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
