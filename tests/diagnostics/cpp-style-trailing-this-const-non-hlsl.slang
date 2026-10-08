//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// The parser preserves trailing `const` outside HLSL so that semantic checking can both explain
// the dialect mismatch and honor the requested non-mutating receiver for the rest of compilation.

struct Counter
{
    int value;

    void reset() const
    /*CHECK:
                 ^^^^^ trailing `const` is only allowed in the HLSL dialect
                 ^^^^^ C++-style trailing `const` requires the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`); in Slang, use `[nonmutating]` or `[mutating]` to state receiver mutability.
    */
    {
        value = 0;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
    }
}
