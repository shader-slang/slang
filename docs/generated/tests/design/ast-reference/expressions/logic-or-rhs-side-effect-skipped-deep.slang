//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=Stresses LogicOperatorShortCircuitExpr `||`: a leading `true` must skip three chained side-effecting right operands.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Deeply chained `||`. Once the leftmost operand is true, none of
// the three subsequent calls to `bump` may execute.

//TEST:INTERPRET(filecheck=CHECK):

bool bump(inout int c) { c++; return false; }

void main()
{
    int c = 0;
    bool _ = (true) || bump(c) || bump(c) || bump(c);
    // CHECK: c=0
    printf("c=%d\n", c);
}
