//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Implicit-conversion ranking inserts an int-to-float coercion at a typed initialization site so the value preserves the source value as float.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Files table assigns slang-check-conversion.cpp the
// responsibility for "implicit-conversion ranking and coercion
// site checks". Verify the user-visible consequence: assigning an
// int to a float local converts losslessly.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 7;
    float f = i;
    // CHECK: f=7
    printf("f=%g\n", f);
}
