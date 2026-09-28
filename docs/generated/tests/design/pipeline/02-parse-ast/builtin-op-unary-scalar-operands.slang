//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
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
