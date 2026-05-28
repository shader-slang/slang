//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Statement checking rejects `case` outside a `switch` statement.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Statement-checking enforces switch-scope rules on case labels.
// Verify a stray `case` is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    case 1:
/*CHECK:
    ^^^^ 'case' not allowed outside of a 'switch' statement
*/
//CHECK: E39999
    int x = 0;
}
