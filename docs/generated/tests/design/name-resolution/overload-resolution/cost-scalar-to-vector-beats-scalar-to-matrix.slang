//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_ScalarToVector (2) is cheaper than kConversionCost_ScalarToMatrix (10), so a float scalar prefers the float2 overload over the float2x2 one.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two additive shape-promotion rows of the cost table against
// each other. A float scalar can broadcast into either a vector (2) or a
// matrix (10); CHECK asserts the vector overload's tag, which is only reachable
// if the vector promotion is the cheaper of the two.

//TEST:INTERPRET(filecheck=CHECK):

int pick(float2 v)   { return 1; }
int pick(float2x2 m) { return 2; }

void main()
{
    float f = 1.0f;
    int tag = pick(f);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
