//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-extension.md#enum
//META: doc_section_digest=6035f3897673a5155e982266b6248994b5a9dc440e9c8a53624caf637fcb81ab
//META: purpose=An `extension` of an enum adds static data and static functions accessed via `EnumType.member`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

enum TestEnum { V1 = 1, V2, V3, V4 }

extension TestEnum
{
    static int staticValue = 42;
    static int staticFunction() { return 123; }
}

void main()
{
    int sv = TestEnum.staticValue;
    int sf = TestEnum.staticFunction();
    //CHECK: sv=42 sf=123
    printf("sv=%d sf=%d\n", sv, sf);
}
