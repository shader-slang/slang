//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A member function implementation is compatible with an interface requirement when the impl function can be called with the interface parameter types and the return type is convertible to the interface return type; wider parameter types and narrower return types are accepted.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "A member function implementation is compatible with an
// interface member function when: the implementation function can be called
// with the parameter types of the interface; AND the implementation function
// return type can be converted to the interface function return type."
// This is the "covariant return / contravariant parameter" compatibility rule.
// Verifies with the doc's own Remark 1 example: interface requires
// int32_t someFunc(int8_t, int16_t) and the impl uses int16_t someFunc(int, int32_t).

//TEST:INTERPRET(filecheck=CHECK):

interface IBase
{
    int32_t someFunc(int8_t a, int16_t b);
}

struct Test : IBase
{
    // Compatible: int16_t (impl) -> int32_t (interface) is OK
    // int (impl param) accepts int8_t (interface param) is OK
    int16_t someFunc(int a, int32_t b)
    {
        return int16_t(a + b);
    }
}

void main()
{
    Test obj;
    int32_t result = obj.someFunc(int8_t(3), int16_t(4));
    //CHECK: result=7
    printf("result=%d\n", result);
}
