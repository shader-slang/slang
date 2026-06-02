//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#syntax
//META: doc_section_digest=362ffce5f577a08a16fa66e283951f5a11bdf34ab8552fa1aa839abf43be9765
//META: purpose=A struct may have generic type parameters; instantiating the generic at different type arguments creates distinct struct types.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The struct-decl grammar includes [generic-params-decl], meaning structs can be generic.
// A generic struct with one type parameter T wrapping a value of type T.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T>
{
    T value;
}

void main()
{
    Box<int> ibox;
    ibox.value = 42;

    Box<float> fbox;
    fbox.value = 3.14f;

    // CHECK: 42
    printf("%d\n", ibox.value);
    // CHECK: 3
    printf("%.0f\n", fbox.value);
}
