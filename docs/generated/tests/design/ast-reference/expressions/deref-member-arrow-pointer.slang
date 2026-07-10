//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr
//META: doc_section_digest=58d3dc1b167dae22620293ef4a1c128bfcf79423ded9a56ab1bbc40bc734de7e
//META: purpose=The `->` surface form accesses a member through a pointer-like base: `p->b` on a struct pointer reads the pointee's field.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `->` member-access surface form on a pointer-like base.
// A uniform Rec* points at a two-field record fed with {7, 42}; reading
// `p->a` and `p->b` must produce the pointee's field values, which CHECK
// asserts from the output buffer. Runs on CPU, where raw pointers are
// supported.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Rec
{
    int a;
    int b;
}

//TEST_INPUT: set p = ubuffer(data=[7 42], stride=4)
uniform Rec* p;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 7
    outputBuffer[0] = p->a;
    // CHECK: 42
    outputBuffer[1] = p->b;
}
