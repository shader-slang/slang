//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=c1890a75acdc226411b7d61f4ee2807ae95bd2bb75bf036bb2da6f27662ff946
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
