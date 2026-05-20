//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=9163d71b0ed4735c5c28e2877570515476519666c5a6da87256143a8fab12f94
//META: purpose=Modifier validation enforces the immutability of `let`-bound locals; assigning to one is rejected as a non-lvalue.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Modifier validation section says modifier-specific checks
// live in slang-check-modifier.cpp. The `let` immutability marker
// surfaces here as a non-lvalue rejection on assignment.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    let x = 5;
    x = 10;
/*CHECK:
      ^ left of '=' is not an l-value
*/
}
