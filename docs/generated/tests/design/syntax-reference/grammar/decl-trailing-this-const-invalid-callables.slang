//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:48:26Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=7bc481f1925fc74337357f76e854855a3f8e8c8a24448fe55a29ec6c5a43f576
//META: purpose=A trailing `const` on a constructor, accessor, or subscript is diagnosed and parsing resumes after the suffix.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK_CTOR): -lang hlsl docs/generated/tests/design/syntax-reference/grammar/decl-trailing-this-const-invalid-callables.slang -DTEST_CASE=1 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK_ACCESSOR): -lang hlsl docs/generated/tests/design/syntax-reference/grammar/decl-trailing-this-const-invalid-callables.slang -DTEST_CASE=2 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK_SUBSCRIPT): -lang hlsl docs/generated/tests/design/syntax-reference/grammar/decl-trailing-this-const-invalid-callables.slang -DTEST_CASE=3 -no-codegen

#if TEST_CASE == 1
struct ConstructorExample
{
    __init() const
//CHECK_CTOR:^^^^^ trailing `const` is not valid on this declaration
//CHECK_CTOR:^^^^^ trailing `const` is only valid on an ordinary function declaration
    {}
};
#endif

#if TEST_CASE == 2
struct AccessorExample
{
    property int value
    {
        get() const
/*CHECK_ACCESSOR:
              ^^^^^ trailing `const` is not valid on this declaration
              ^^^^^ trailing `const` is only valid on an ordinary function declaration
*/
        {
            return 0;
        }
    }
};
#endif

#if TEST_CASE == 3
struct SubscriptExample
{
    __subscript(int index) const -> int
//CHECK_SUBSCRIPT:         ^^^^^ trailing `const` is not valid on this declaration
//CHECK_SUBSCRIPT:         ^^^^^ trailing `const` is only valid on an ordinary function declaration
    {
        get
        {
            return index;
        }
    }
};
#endif
