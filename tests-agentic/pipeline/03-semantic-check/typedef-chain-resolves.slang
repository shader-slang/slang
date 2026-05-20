//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Type resolution follows a chain of `typealias` declarations and resolves all the way to the underlying primitive type.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on typedef-resolves-to-base-type: a 3-step alias chain
// still resolves correctly to the underlying `int`.

//TEST:INTERPRET(filecheck=CHECK):

typealias A = int;
typealias B = A;
typealias C = B;

void main()
{
    C x = 7;
    // CHECK: x=7
    printf("x=%d\n", x);
}
