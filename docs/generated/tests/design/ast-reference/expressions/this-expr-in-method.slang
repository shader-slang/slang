//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=26c797434ab51700cf84e595593cca11917ffceaff2d95c7a0504524e997316e
//META: purpose=A ThisExpr inside a member function refers to the enclosing aggregate's instance; reads through `this` see the receiver's state.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ThisExpr is listed in the Nodes table as `this` of the enclosing
// aggregate type. Inside a struct method, `this.field` and `field`
// both name the receiver's field; reading `this.x` returns the
// receiver's `x`.

//TEST:INTERPRET(filecheck=CHECK):

struct Box
{
    int side;
    int doubleSide() { return this.side * 2; }
}

void main()
{
    Box b = {7};
    // CHECK: r=14
    printf("r=%d\n", b.doubleSide());
}
