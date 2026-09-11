//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_SizedArrayToUnsizedArray (30) makes an unsized-array parameter lose to an exactly-sized one for a fixed-size array argument.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the sized-to-unsized-array row of the cost table. An int[3]
// argument reaches int[] by paying 30 and int[3] for free, so the exactly
// sized candidate wins; CHECK asserts its tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int a[])  { return 1; }
int pick(int a[3]) { return 2; }

void main()
{
    int arr[3] = { 1, 2, 3 };
    int tag = pick(arr);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
