//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A struct may conform to one or more interface types; the struct implements the interface requirements and can be used wherever the interface is expected.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A structure may conform to one or more interface types."
// MyStruct conforms to IHasValue and provides the getValue() method required by the interface.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasValue
{
    int getValue();
}

struct MyStruct : IHasValue
{
    int data;

    int getValue()
    {
        return data;
    }
}

int readValue<T : IHasValue>(T obj)
{
    return obj.getValue();
}

void main()
{
    MyStruct s;
    s.data = 77;
    int v = readValue(s);
    // CHECK: 77
    printf("%d\n", v);
}
