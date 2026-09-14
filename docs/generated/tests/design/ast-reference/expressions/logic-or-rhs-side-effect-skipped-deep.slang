//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
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
