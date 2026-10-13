//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A Slang class always uses `in` for its implicit receiver. Semantic checking rejects
// receiver-mode specifiers on its methods and separately diagnoses the HLSL-only trailing syntax
// because this test uses the Slang dialect.

class Reference
{
    void method() const
    /*CHECK:
                  ^^^^^ trailing `const` is only allowed in the HLSL dialect
                  ^^^^^ C++-style trailing `const` requires the HLSL dialect (use a `.hlsl` file or pass `-lang hlsl`).
                  ^^^^^ receiver-mode specifier is not allowed on a class member
                  ^^^^^ receiver-mode specifier 'const' is not allowed on a class member; its implicit receiver uses the `in` parameter-passing mode.
    */
    {}
};
