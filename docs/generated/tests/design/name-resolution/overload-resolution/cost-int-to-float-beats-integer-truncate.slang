//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=d77239a753c95159f4db29b226999df363503e43e2ce42ec3d20143496696e2b
//META: purpose=kConversionCost_IntegerToFloatConversion (400) is cheaper than kConversionCost_IntegerTruncate (450), so an int argument prefers a float parameter over a narrower integer one.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the near-neighbour pair in the cost table: crossing to float is
// ranked cheaper than narrowing within the integer family. Both overloads are
// viable for an int argument; CHECK asserts the float one fired.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int8_t x) { return 1; }
int pick(float x)  { return 2; }

void main()
{
    int a = 5;
    int tag = pick(a);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
