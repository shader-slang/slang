//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#d3d-constant-buffer-layout
//META: doc_section_digest=ebd30924bf777b67b6ac3c520785d923dfbd7247c96b9f0e8010016332107ae9
//META: purpose=Inside a struct body, `This` refers to the enclosing struct type itself and can be used in method return types and local variable declarations.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `This` inside a struct body resolves to the enclosing struct type.
// withValue() declares its return type and local variable as `This`; the interpreter
// executes the call and CHECK confirms the returned instance carries the expected value.

//TEST:INTERPRET(filecheck=CHECK):

struct MyStruct
{
    int value;

    // `This` used as return type -- returns a new instance
    This withValue(int v)
    {
        This result;
        result.value = v;
        return result;
    }
}

void main()
{
    MyStruct s;
    s.value = 0;
    MyStruct s2 = s.withValue(99);
    // CHECK: 99
    printf("%d\n", s2.value);
}
