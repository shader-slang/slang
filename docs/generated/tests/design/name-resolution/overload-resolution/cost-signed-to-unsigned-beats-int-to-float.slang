//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_SignedToUnsignedConversion (250) is cheaper than kConversionCost_IntegerToFloatConversion (400), so an int variable selects the uint overload.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the signed-to-unsigned row of the cost table. The argument is a
// variable rather than a literal so the literal-specific levels do not apply:
// int reaches uint at 250 and float at 400. CHECK asserts the uint tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(uint x)  { return 1; }
int pick(float x) { return 2; }

void main()
{
    int v = 5;
    int tag = pick(v);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
