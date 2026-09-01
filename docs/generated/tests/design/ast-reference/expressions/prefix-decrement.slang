//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A PrefixExpr `--x` is the prefix decrement operator; its value is the post-decrement value of the operand.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Prefix-form `--x` reduces x by 1 and yields the new value. After
// `int v = --i;` we expect both v and i to be one less than the
// original value of i.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 10;
    int v = --i;
    // CHECK: v=9
    printf("v=%d\n", v);
    // CHECK: i=9
    printf("i=%d\n", i);
}
