//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=An ArrayExpressionType carries an element type and an element count; each of the N slots of a sized array is independently readable and writable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the sized form of the array type: an int[4] has four
// independent slots. Each is written with a distinct value and read
// back; CHECK asserts all four, so an off-by-one count or aliased slots
// would be visible.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[4];
    for (int i = 0; i < 4; ++i) { a[i] = i * 3; }
    // CHECK: arr=0,3,6,9
    printf("arr=%d,%d,%d,%d\n", a[0], a[1], a[2], a[3]);
}
