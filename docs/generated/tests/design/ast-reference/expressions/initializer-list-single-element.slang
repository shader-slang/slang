//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
