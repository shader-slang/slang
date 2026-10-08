//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A Slang class is a reference type whose methods always use `in` mode for `this` and reject
// receiver-mode specifiers. C++-style trailing `const` is also HLSL-only syntax, so this method
// diagnoses both rules.

class Reference
{
    void method() const
    /*CHECK:
                  ^^^^^ trailing `const` is only allowed in the HLSL dialect
                  ^^^^^ C++-style trailing `const` requires the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`); in Slang, use `[nonmutating]` or `[mutating]` to state receiver mutability.
                  ^^^^^ receiver-mode specifier is not allowed on a class member
                  ^^^^^ receiver-mode specifier 'const' is not allowed on a class member; a class receiver always uses the `in` parameter-passing mode.
    */
    {}
};
