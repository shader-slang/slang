//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=d86992208065a1cd06198a0c8298546dee4838c3ae7ea2c807513713ba87455a
//META: purpose=The builtin-operator fast path covers bool scalar operands: `==` and `!=` on bool values yield the expected truth values.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that builtin comparison operators accept bool scalar operands,
// one of the documented operand categories of the fast path. `true == false`
// and `true != false` are evaluated and CHECK asserts the 0/1 results, so
// a mis-mapped operation kind would flip the observed values.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool t = true;
    bool f = false;
    // CHECK: eq=0
    printf("eq=%d\n", int(t == f));
    // CHECK: ne=1
    printf("ne=%d\n", int(t != f));
}
