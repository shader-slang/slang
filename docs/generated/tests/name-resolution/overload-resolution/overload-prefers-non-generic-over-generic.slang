//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=cf4ad761e64c7c27fa923b073fa264eed8e3a5152ac4e063d5e36eba8cb6323b
//META: purpose=The comparator's compareOverloadCandidateSpecificity step prefers a non-generic candidate over a generic one when both are Applicable with the same conversion cost.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "compareOverloadCandidateSpecificity ... non-generic > generic".
// Both overloads accept an int with identity cost; the comparator
// step 5 picks the non-generic one.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)    { return 1; }     // non-generic
int pick<T>(T x)   { return 2; }     // generic

void main()
{
    int tag = pick(42);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
