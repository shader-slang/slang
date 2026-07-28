//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=6195cb94dbbd650ed3781180b621c12cc25bdf23da298eb8248708408790b4b2
//META: purpose=An exact-type match (cost 0) beats a scalar-to-vector promotion (kConversionCost_ScalarToVector additive); the int3 overload is not chosen when the call passes an int.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two overloads: one takes `int`, the other takes `int3`. Calling
// with an int, the int overload's conversionCostSum is 0 while the
// int3 overload requires ScalarToVector promotion. The int overload
// wins.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)  { return 1; }
int pick(int3 v) { return 3; }

void main()
{
    int tag = pick(7);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
