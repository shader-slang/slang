//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An IndexExpr can chain `a[i][j]` for multi-dimensional indexing; the result is the (i,j)-th element.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says IndexExpr supports "(one or more indices)". A 2D
// array indexed twice reads the element at the chained position;
// `a[i][j]` is parsed as nested IndexExpr nodes.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int grid[2][3] = { {1, 2, 3}, {4, 5, 6} };
    // CHECK: g00=1
    printf("g00=%d\n", grid[0][0]);
    // CHECK: g12=6
    printf("g12=%d\n", grid[1][2]);
}
