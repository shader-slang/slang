//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=ArrayExpressionType scales beyond the small smoke-test sizes; an int[16] correctly holds 16 independent elements and the bookends are observable.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: a larger ArrayExpressionType (int[16]) populated
// with i*i values. We observe the lower-edge (index 0) and the
// upper-edge (index 15) to confirm the array's element-count
// operand survives at the documented size.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[16];
    for (int i = 0; i < 16; ++i) a[i] = i * i;
    // CHECK: lo=0
    printf("lo=%d\n", a[0]);
    // CHECK: hi=225
    printf("hi=%d\n", a[15]);
}
