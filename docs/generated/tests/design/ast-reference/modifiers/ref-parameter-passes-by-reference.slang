//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#reference-parameter-keywords
//META: doc_section_digest=b27e1faf6e9ecd77be8bf939451515b867e905561ac61f1289d503aca4311f21
//META: purpose=A `ref` parameter (accepted spelling `__ref`) aliases the caller's storage rather than copying it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `ref` parameter-passing mode: unlike `in`, the callee operates
// on the caller's storage directly. `bump` adds 5 through the reference and
// CHECK asserts the caller reads 6, so the write reached the caller's variable.
// The accepted surface spelling is `__ref`, not the bare `ref` the doc names --
// see the README's doc-gap row.

//TEST:INTERPRET(filecheck=CHECK):

void bump(__ref int x)
{
    x = x + 5;
}

void main()
{
    int v = 1;
    bump(v);
    //CHECK: v=6
    printf("v=%d\n", v);
}
