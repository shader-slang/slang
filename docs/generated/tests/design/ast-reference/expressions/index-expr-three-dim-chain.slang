//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Stresses chained IndexExpr nodes: `arr[i][j][k]` on a 3-D fixed-size array reads back the assigned element.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Three-deep chain of IndexExpr (the surface for multi-dim arrays).
// Each `[]` is its own IndexExpr whose base is the previous one.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[2][2][2];
    arr[0][0][0] = 1;
    arr[1][1][1] = 8;
    arr[0][1][0] = 3;
    // CHECK: a=1 b=8 c=3
    printf("a=%d b=%d c=%d\n", arr[0][0][0], arr[1][1][1], arr[0][1][0]);
}
