//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
