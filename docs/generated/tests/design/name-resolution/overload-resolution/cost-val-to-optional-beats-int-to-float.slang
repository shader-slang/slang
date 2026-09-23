//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=ac2c2ef8658efb348430e3e014feb776a1624731934544b00f6fa4d97ab710e8
//META: purpose=kConversionCost_ValToOptional (150) is cheaper than kConversionCost_IntegerToFloatConversion (400), so an int argument selects the Optional<int> overload.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the T-to-Optional row of the cost table. An int argument can wrap
// into Optional<int> at 150 or widen to float at 400, so the Optional
// candidate wins; CHECK asserts its tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(Optional<int> x) { return 1; }
int pick(float x)         { return 2; }

void main()
{
    int v = 5;
    int tag = pick(v);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
