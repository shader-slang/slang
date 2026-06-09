//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=8135956851ca195027226ec4800f952b5b9939ca3939bc07d946ccdb02975fc6
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
