//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=ArrayExpressionType at the smallest element-count (N=1) is still a valid sized array; the single element is independently addressable.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the small edge of ArrayExpressionType: an
// `int[1]` is a one-element sized array. The (element-type,
// element-count) operand pair degenerates to a singleton.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[1];
    a[0] = 99;
    // CHECK: a0=99
    printf("a0=%d\n", a[0]);
}
