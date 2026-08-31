//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=An identity match (kConversionCost_None, 0) beats kConversionCost_IntegerToFloatConversion (400) when both overloads accept an int argument.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the cost step at its cheapest point. `pick(int)` costs 0 for an int
// argument while `pick(float)` costs 400; the comparator's lower-sum rule picks
// the int overload. CHECK asserts its tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)   { return 1; }
int pick(float x) { return 2; }

void main()
{
    int a = 7;
    int tag = pick(a);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
