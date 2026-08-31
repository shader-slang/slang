//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_PtrToBool (400) makes a pointer argument lose to an exact Ptr parameter while still being convertible to bool.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pointer-to-bool row of the cost table. Taking the address of a
// local produces a Ptr<int> that both overloads accept, but the bool candidate
// pays 400 against the exact match's 0; CHECK asserts the pointer tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(Ptr<int> p) { return 1; }
int pick(bool b)     { return 2; }

void main()
{
    int x = 5;
    int tag = pick(&x);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
