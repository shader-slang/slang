//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-extension.md#struct
//META: doc_section_digest=6035f3897673a5155e982266b6248994b5a9dc440e9c8a53624caf637fcb81ab
//META: purpose=Doc Example 3 (struct): an `extension TestClass : IReq` ADDS a new interface conformance to a struct that did not originally declare it.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Unlike Example 2 (where the struct declared `: IReq` but didn't
// implement), here the struct does NOT declare conformance at
// definition. The extension adds both the conformance and the
// implementation.

//TEST:INTERPRET(filecheck=CHECK):

interface IReq
{
    int requiredFunc();
}

struct TestClass
{
}

extension TestClass : IReq
{
    int requiredFunc()
    {
        return 42;
    }
}

void main()
{
    TestClass concrete = {};
    int r = concrete.requiredFunc();
    //CHECK: r=42
    printf("r=%d\n", r);
}
