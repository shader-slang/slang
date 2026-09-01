//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_OneVectorToScalar (1) makes a one-element vector argument reach a scalar parameter implicitly.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one-vector-to-scalar row of the cost table. A vector<float,1>
// argument is accepted by a plain float parameter, which is only possible if
// the downcast is a permitted implicit conversion; CHECK asserts the value the
// callee computed from the downcast element.

//TEST:INTERPRET(filecheck=CHECK):

int pick(float x) { return int(x) + 1; }

void main()
{
    vector<float, 1> v = vector<float, 1>(41.0f);
    int tag = pick(v);
    // CHECK: tag=42
    printf("tag=%d\n", tag);
}
