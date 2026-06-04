//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
