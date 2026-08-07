//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies brace aggregate initializers for a struct and a fixed-size array lower so each member/element gets its listed value.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins aggregate-initializer lowering for `P p = {3,4}` and
// `int arr[3] = {10,20,30}`: the struct's positional fields map to x=3,y=4
// and the array elements map by index. Reads back p.x+p.y, arr[0], arr[2].
// INTERPRET; no short-circuit operators so the slangi VM path is reliable.
//TEST:INTERPRET(filecheck=CHECK):
struct P { int x; int y; }
void main()
{
    P p = { 3, 4 };
    int arr[3] = { 10, 20, 30 };
    printf("%d %d %d\n", p.x + p.y, arr[0], arr[2]);
    //CHECK: 7 10 30
}
