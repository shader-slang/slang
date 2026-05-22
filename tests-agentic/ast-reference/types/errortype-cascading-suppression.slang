//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#errortype-and-bottomtype
//META: doc_section_digest=e1094706189ea5753694864d309f989781ef17b6614c66fc930e727c94224319
//META: purpose=ErrorType is the type of expressions that failed checking; downstream uses of a failed expression do not produce cascading type-mismatch diagnostics.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `ErrorType` note states: "the type of any expression
// that failed to check, so that downstream checking does not
// cascade". A single undeclared-identifier should produce ONE
// diagnostic — the downstream arithmetic uses of the ErrorType
// value must not each emit additional errors.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = bogusName;
//CHECK:    ^^^^^^^^^ undefined identifier 'bogusName'
// CHECK: E30015
    int y = x + 1;
    int z = x * 2;
    int w = y + z;
}
