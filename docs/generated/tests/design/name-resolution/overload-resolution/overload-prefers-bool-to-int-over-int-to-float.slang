//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=6195cb94dbbd650ed3781180b621c12cc25bdf23da298eb8248708408790b4b2
//META: purpose=Conversion-cost table places kConversionCost_BoolToInt (120) deliberately cheaper than kConversionCost_IntegerToFloatConversion (400); when both overloads are callable with a bool argument, the int overload wins.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's conversion-cost table marks BoolToInt at 120 "(deliberately
// cheaper to break ties)" against IntegerToFloatConversion at 400.
// Calling `pick(true)` makes both overloads reachable, but the int
// overload has lower conversionCostSum and wins.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)   { return 1; }
int pick(float x) { return 2; }

void main()
{
    int tag = pick(true);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
