//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_NoneToOptional (150) lets none reach an Optional parameter, so the Optional overload is the one the resolver selects.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the none-to-Optional row of the cost table. `none` has its own type
// and the only conversion the table offers for it is into an Optional, so the
// float candidate is dropped at the type step and the Optional candidate is
// chosen; CHECK asserts the Optional overload's tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(Optional<int> x) { return 1; }
int pick(float x)         { return 2; }

void main()
{
    int tag = pick(none);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
