//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A Slang class always uses `in` for its implicit receiver and rejects receiver-mode specifiers.
// Because this test uses the Slang dialect, trailing `const` also diagnoses its HLSL-only syntax.

class Reference
{
    void method() const
    /*CHECK:
                  ^^^^^ trailing `const` is only allowed in the HLSL dialect
                  ^^^^^ C++-style trailing `const` requires the HLSL dialect (use a `.hlsl` file or pass `-lang hlsl`); in Slang, use `[nonmutating]` or `[mutating]` to state receiver mutability.
                  ^^^^^ receiver-mode specifier is not allowed on a class member
                  ^^^^^ receiver-mode specifier 'const' is not allowed on a class member; its implicit receiver uses the `in` parameter-passing mode.
    */
    {}
};
