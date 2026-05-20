//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=ArrayExpressionType nests: int[2][3] is an array-of-arrays whose element-type is itself an ArrayExpressionType.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the multi-dimensional axis: `int[2][3]` is an
// array-of-arrays. The outer ArrayExpressionType has element-type
// = inner ArrayExpressionType (int[3]) and element-count = 2.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[2][3];
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 3; ++c)
            a[r][c] = r * 10 + c;
    // CHECK: tl=0
    printf("tl=%d\n", a[0][0]);
    // CHECK: tr=2
    printf("tr=%d\n", a[0][2]);
    // CHECK: bl=10
    printf("bl=%d\n", a[1][0]);
    // CHECK: br=12
    printf("br=%d\n", a[1][2]);
}
