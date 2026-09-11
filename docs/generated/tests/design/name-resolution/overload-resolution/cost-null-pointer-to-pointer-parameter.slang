//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_NullPtrToPtr (150) makes nullptr reach a Ptr parameter, selecting the pointer overload over an int one.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the nullptr-to-pointer row of the cost table. `nullptr` has its own
// NullPtr type, so only the Ptr candidate survives the type step; CHECK
// asserts the pointer overload's tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(Ptr<int> p) { return 1; }
int pick(int v)      { return 2; }

void main()
{
    int tag = pick(nullptr);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
