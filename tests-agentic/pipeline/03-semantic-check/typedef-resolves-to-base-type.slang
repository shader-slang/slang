//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Type checking resolves a typedef'd name to its base type so an operation on the alias is the same as on the underlying type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slang-check-type.cpp resolves type references. Verify the
// user-visible consequence by computing through a typealias and
// confirming the result matches the int form.

//TEST:INTERPRET(filecheck=CHECK):

typealias U32 = uint;

void main()
{
    U32 n = 5;
    uint r = n + 3;
    // CHECK: r=8
    printf("r=%u\n", r);
}
