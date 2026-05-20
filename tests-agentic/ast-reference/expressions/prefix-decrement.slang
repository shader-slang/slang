//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
