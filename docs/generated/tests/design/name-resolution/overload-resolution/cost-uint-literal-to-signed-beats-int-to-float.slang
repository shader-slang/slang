//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_InRangeIntLitUnsignedToSignedConversion (81) is cheaper than kConversionCost_IntegerToFloatConversion (400), so an unsigned literal selects the int overload.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unsigned-literal-to-signed-target row of the cost table. The
// literal 5u reaches the int parameter at 81 and the float parameter at 400,
// so the cheaper int candidate wins; CHECK asserts its tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)   { return 1; }
int pick(float x) { return 2; }

void main()
{
    int tag = pick(5u);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
