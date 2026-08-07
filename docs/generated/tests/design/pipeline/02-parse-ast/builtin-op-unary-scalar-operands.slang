//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
//META: purpose=Builtin unary operators (arithmetic negate, logical not, bitwise not) over builtin scalar operands are rewritten by the checker and evaluate correctly.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the checker's builtin-operator rewrite covers the unary
// operator class. Each unary form (`-`, `!`, `~`) is applied to a
// runtime value from a function call; CHECK asserts the negated,
// logically-inverted, and bit-inverted results.

//TEST:INTERPRET(filecheck=CHECK):

int pass(int v)
{
    return v;
}

void main()
{
    int x = pass(7);
    bool t = x > 0;
    uint u = uint(pass(0));
    // CHECK: neg=-7 not=0 bnot=4294967295
    printf("neg=%d not=%d bnot=%u\n", -x, int(!t), ~u);
}
