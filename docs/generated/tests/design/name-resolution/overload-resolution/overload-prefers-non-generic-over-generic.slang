//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=dfab71819770e1c3116c8ab890c4c05608d82d83fe8f00436fcbffa88f52900d
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
