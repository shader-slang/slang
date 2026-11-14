//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Stresses SelectExpr: a three-deep nested `c ? a : (c' ? a' : (c'' ? a'' : b''))` ternary chain must select the matching arm.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Three nested SelectExpr nodes. The parser must associate the
// right-hand `?:` chains as a tower, and the evaluator must pick
// the arm whose preceding condition is true.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 2;
    int r = (x == 0) ? 100 : (x == 1) ? 200 : (x == 2) ? 300 : 400;
    // CHECK: r=300
    printf("r=%d\n", r);
}
