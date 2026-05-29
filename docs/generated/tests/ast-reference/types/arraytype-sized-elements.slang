//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=An ArrayExpressionType carries an element-type and an element-count; each of the N elements is independently readable and writable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ArrayExpressionType` is a DeclRefType-descendant for sized /
// unsized arrays. The user-facing claim: an `int[4]` is a sized
// array; writing each element and reading it back gives the
// expected value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[4];
    for (int i = 0; i < 4; ++i) { a[i] = i * 3; }
    // CHECK: arr=0,3,6,9
    printf("arr=%d,%d,%d,%d\n", a[0], a[1], a[2], a[3]);
}
