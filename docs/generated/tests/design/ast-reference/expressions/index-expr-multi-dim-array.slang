//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=An IndexExpr can chain `a[i][j]` for multi-dimensional indexing; the result is the (i,j)-th element.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that subscripting can be chained: `a[i][j]` reaches the element of
// the inner array. Distinct values are stored at two positions and read back
// through two-level subscripts, so a collapsed or transposed index shows up as
// a wrong value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int grid[2][3] = { {1, 2, 3}, {4, 5, 6} };
    // CHECK: g00=1
    printf("g00=%d\n", grid[0][0]);
    // CHECK: g12=6
    printf("g12=%d\n", grid[1][2]);
}
