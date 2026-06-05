//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface may be parameterized with generics; a struct conforms by providing a concrete type argument.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a struct can conform to a generic interface instantiated with
// a concrete type argument. TestClass conforms to ITypedReq<uint> and its
// someFunc adds 123 to the parameter; CHECK asserts result=130, proving the
// generic interface was correctly specialised and dispatched.

//TEST:INTERPRET(filecheck=CHECK):

interface ITypedReq<T>
{
    T someFunc(T param);
}

struct TestClass : ITypedReq<uint>
{
    uint someFunc(uint param)
    {
        return 123 + param;
    }
}

void main()
{
    TestClass obj;
    uint res = obj.someFunc(7);
    //CHECK: result=130
    printf("result=%d\n", res);
}
