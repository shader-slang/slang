//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/diagnostics/cpp-style-trailing-this-const-non-hlsl.slang -no-codegen

// The parser preserves trailing `const` in Slang and GLSL mode. Semantic checking diagnoses the
// dialect mismatch, but still uses a read-only receiver while checking the method body.

struct Counter
{
    int value;

    void reset() const
    /*CHECK:
                 ^^^^^ trailing `const` is only allowed in the HLSL dialect
                 ^^^^^ C++-style trailing `const` requires the HLSL dialect (use a `.hlsl` file or pass `-lang hlsl`).
                 ^^^^^ use Slang's `[nonmutating]` or `[mutating]` receiver-mode attribute instead
    */
    {
        value = 0;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
    }

    void repeatedTrailingConst() const const
    /*CHECK:
                                 ^^^^^ trailing `const` is only allowed in the HLSL dialect
                                 ^^^^^ C++-style trailing `const` requires the HLSL dialect (use a `.hlsl` file or pass `-lang hlsl`).
                                       ^^^^^ trailing `const` is only allowed in the HLSL dialect
                                       ^^^^^ C++-style trailing `const` requires the HLSL dialect (use a `.hlsl` file or pass `-lang hlsl`).
                                 ^^^^^ use Slang's `[nonmutating]` or `[mutating]` receiver-mode attribute instead
                                       ^^^^^ duplicate modifier
                                       ^^^^^ modifier 'const' is redundant or conflicting with existing modifier 'const'
    */
    {}
};
