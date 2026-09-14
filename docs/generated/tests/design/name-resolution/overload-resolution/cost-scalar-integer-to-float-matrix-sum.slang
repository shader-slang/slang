//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_ScalarIntegerToFloatMatrix (410) is the sum of the int-to-float and scalar-to-matrix levels, so an int scalar prefers an int2x2 parameter (10) over a float2x2 one.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the composite 410 level really is 400 + 10 rather than a
// cheaper special case. An int scalar broadcast into int2x2 pays only the
// scalar-to-matrix 10, while float2x2 pays 410; CHECK asserts the int-matrix
// overload's tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(float2x2 m) { return 1; }
int pick(int2x2 m)   { return 2; }

void main()
{
    int v = 3;
    int tag = pick(v);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
