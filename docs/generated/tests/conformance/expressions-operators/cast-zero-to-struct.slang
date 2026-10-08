//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#compatibility-feature
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Compatibility cast `(StructTy)0` is equivalent to `{}` initialization per the doc.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's #compatibility-feature section states explicitly that
// `(MyStruct)0` is "equivalent to initialization from an empty
// initializer list", i.e. `{}`. Verify both forms produce a struct
// whose fields read the documented zero/default values.

//TEST:INTERPRET(filecheck=CHECK):

struct MyStruct
{
    int a;
    float b;
};

void main()
{
    MyStruct s1 = (MyStruct)0;
    MyStruct s2 = {};
    //CHECK: s1.a=0 s1.b=0 s2.a=0 s2.b=0
    printf("s1.a=%d s1.b=%d s2.a=%d s2.b=%d\n",
           s1.a, (int)s1.b, s2.a, (int)s2.b);
}
