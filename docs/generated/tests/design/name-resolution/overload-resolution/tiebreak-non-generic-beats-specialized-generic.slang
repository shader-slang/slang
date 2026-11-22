//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=764e196b19a5dd0e0f2c516c498be250f0fdaba7e58c28f2ac820f88be1e5466
//META: purpose=compareOverloadCandidateSpecificity ranks a candidate that is the inner declaration of a generic below one that is not, so a non-generic candidate beats a generic one of equal cost.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the specificity step. Both overloads accept an int with identity
// cost, so cost cannot separate them; the generic candidate is the inner declaration of
// a generic and the non-generic one is not. CHECK asserts the non-generic tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x)  { return 1; }
int pick<T>(T x) { return 2; }

void main()
{
    int tag = pick(42);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
