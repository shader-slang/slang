//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:48:26Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#mutability--autodiff-annotations
//META: doc_section_digest=205f14f37aca487580978a6b28c97a245206a01340f74658dbbf2e0dc7d8e7fb
//META: purpose=HLSL trailing `const` selects a read-only configurable value-type receiver, while a Slang class rejects the receiver-mode specifier and retains its fixed `in` receiver.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK_HLSL,non-exhaustive): -lang hlsl docs/generated/tests/design/ast-reference/modifiers/cpp-style-trailing-this-const-selects-read-only-receiver.slang -DTEST_CLASS=0 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK_CLASS): docs/generated/tests/design/ast-reference/modifiers/cpp-style-trailing-this-const-selects-read-only-receiver.slang -DTEST_CLASS=1 -no-codegen

#if TEST_CLASS
class ReferenceValue
{
    void reset() const
    /*CHECK_CLASS:
                 ^^^^^ trailing `const` is only allowed in the HLSL dialect
                 ^^^^^ C++-style trailing `const` requires the HLSL dialect (use a `.hlsl` file or pass `-lang hlsl`).
                 ^^^^^ receiver-mode specifier is not allowed on a class member
                 ^^^^^ receiver-mode specifier 'const' is not allowed on a class member; its implicit receiver uses the `in` parameter-passing mode.
    */
    {}
};
#else
struct Value
{
    int field;

    void reset() const
    {
        field = 0;
/*CHECK_HLSL:
        ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
*/
    }
};
#endif
