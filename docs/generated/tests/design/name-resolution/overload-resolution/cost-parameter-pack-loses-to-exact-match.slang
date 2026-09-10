//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_ParameterPack (500) makes binding to a variadic pack lose to an exact non-pack match of cost 0.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the parameter-pack row of the cost table. Both the plain int
// overload and the variadic pack overload accept a single int argument, but
// the pack binding costs 500 against the exact match's 0; CHECK asserts the
// non-pack tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x) { return 1; }
int pick<each T>(expand each T args) { return 2; }

void main()
{
    int v = 3;
    int tag = pick(v);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
