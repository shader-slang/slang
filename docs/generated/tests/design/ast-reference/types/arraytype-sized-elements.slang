//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=acec388be6a124a34086d58d95713bb5cd33eeac4b8fcf6534173cfc8ec11143
//META: purpose=An ArrayExpressionType carries an element-type and an element-count; each of the N elements of a sized array is independently readable and writable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies ArrayExpressionType is a sized array of (element-type,
// count): an int[4] has four independent slots. Each is written then
// read back; CHECK asserts all four values, proving the count-4 shape.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[4];
    for (int i = 0; i < 4; ++i) { a[i] = i * 3; }
    // CHECK: arr=0,3,6,9
    printf("arr=%d,%d,%d,%d\n", a[0], a[1], a[2], a[3]);
}
