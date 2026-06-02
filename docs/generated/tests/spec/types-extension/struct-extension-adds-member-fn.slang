//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-extension.md#struct
//META: doc_section_digest=6035f3897673a5155e982266b6248994b5a9dc440e9c8a53624caf637fcb81ab
//META: purpose=Doc Example 1 (struct): an `extension` adds a member function to a previously-defined struct; the function is callable on instances of the original struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Direct port of the doc's Example 1 for struct extensions.

//TEST:INTERPRET(filecheck=CHECK):

struct ExampleStruct
{
    int a;

    int getASquared()
    {
        return a * a;
    }
}

extension ExampleStruct
{
    [mutating] void addToA(int x)
    {
        a = a + x;
    }
}

void main()
{
    ExampleStruct s = { 3 };
    int sq = s.getASquared();
    s.addToA(10);
    //CHECK: sq=9 a=13
    printf("sq=%d a=%d\n", sq, s.a);
}
