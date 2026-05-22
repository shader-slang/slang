//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Declaration checking rejects two locals with the same name in the same block as a conflicting declaration.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slang-check-decl.cpp handles decl checking, including name-conflict
// detection. Two locals of the same name in the same block must be
// reported.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = 1;
    float x = 2;
//CHECK:  ^ conflicting declaration
//CHECK:  ^ declaration of 'x' conflicts with existing declaration
// CHECK: E30200
}
