//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6165fbd3f46bb23f31d4481667f01c7f841053292ade0127deceffcbed0876dc
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
// CHECK: E30011
}
