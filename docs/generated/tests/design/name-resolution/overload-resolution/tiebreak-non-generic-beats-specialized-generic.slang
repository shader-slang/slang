//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=02381f6488db1154095a241fb46e92defb884509c6c7ee2e2bc10be1811be455
//META: purpose=compareOverloadCandidateSpecificity prefers the smaller getSpecializedParamCount, so a non-generic candidate beats a specialized generic one of equal cost.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the specificity step. Both overloads accept an int with identity
// cost, so cost cannot separate them; the generic candidate carries a
// specialized-parameter count of one against the non-generic candidate's zero.
// CHECK asserts the non-generic tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)  { return 1; }
int pick<T>(T x) { return 2; }

void main()
{
    int tag = pick(42);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
