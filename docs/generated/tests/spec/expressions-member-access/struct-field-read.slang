//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-member-access.md#member-expression
//META: doc_section_digest=ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
//META: purpose=Struct field access via `base.fieldName` reads the named field; multiple fields read independently.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

struct Point { int x; int y; int z; }

void main()
{
    Point p = { 3, 5, 7 };
    //CHECK: x=3 y=5 z=7
    printf("x=%d y=%d z=%d\n", p.x, p.y, p.z);
}
