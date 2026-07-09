//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=6195cb94dbbd650ed3781180b621c12cc25bdf23da298eb8248708408790b4b2
//META: purpose=When two overloads are callable with an int argument, the int->int identity (cost 0) wins over int->float (kConversionCost_IntegerToFloatConversion=400) per the conversion-cost ranking.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two overloads of `pick` exist: one taking `int`, one taking `float`.
// Calling with an int literal, the int overload has conversionCostSum=0
// while the float overload would need an IntegerToFloatConversion (400).
// The comparator's step 2 ("Lower conversionCostSum wins") picks `int`.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)   { return 1; }
int pick(float x) { return 2; }

void main()
{
    int tag = pick(7);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
