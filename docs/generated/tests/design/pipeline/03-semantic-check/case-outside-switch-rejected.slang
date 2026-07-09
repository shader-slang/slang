//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
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
    int x = 0;
}
