//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=8135956851ca195027226ec4800f952b5b9939ca3939bc07d946ccdb02975fc6
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
