//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=d77239a753c95159f4db29b226999df363503e43e2ce42ec3d20143496696e2b
//META: purpose=kConversionCost_BoolToInt (120) is cheaper than kConversionCost_IntegerToFloatConversion (400), so a bool argument selects the int overload.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the deliberately-cheap bool-to-int level. A bool argument can reach
// either overload, but the int one accumulates 120 against the float one's
// 400. CHECK asserts the int overload's tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)   { return 1; }
int pick(float x) { return 2; }

void main()
{
    bool b = true;
    int tag = pick(b);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
