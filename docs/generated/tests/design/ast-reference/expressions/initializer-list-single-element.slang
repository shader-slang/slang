//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=26c797434ab51700cf84e595593cca11917ffceaff2d95c7a0504524e997316e
//META: purpose=Verifies that a single-element InitializerListExpr `{99}` populates the sole field of a one-field aggregate.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimal non-empty boundary for InitializerListExpr.args: exactly
// one element. The positional rule still applies for a single-field
// aggregate.

//TEST:INTERPRET(filecheck=CHECK):

struct One { int v; }

void main()
{
    One o = {99};
    // CHECK: v=99
    printf("v=%d\n", o.v);
}
