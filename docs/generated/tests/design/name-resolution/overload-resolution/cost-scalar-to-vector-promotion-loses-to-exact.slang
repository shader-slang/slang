//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_ScalarToVector is additive on top of the element conversion, so an exact scalar match beats promoting the same argument to a vector.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the scalar-to-vector promotion is a real (nonzero) cost rather
// than a free reshape: with an int argument the int3 overload is viable but
// strictly more expensive than the int one. CHECK asserts the scalar overload
// fired.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)  { return 1; }
int pick(int3 v) { return 3; }

void main()
{
    int a = 7;
    int tag = pick(a);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
