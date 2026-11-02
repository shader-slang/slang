//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// The Slang dialect diagnoses trailing `const`, but preserves its read-only receiver mode while
// checking the rest of the declaration. A Slang-style class also rejects trailing `const`, as it
// rejects every receiver-mode spelling, because its receiver always uses `in`.

struct Counter
{
    int value;

    void reset() const
    /*CHECK:
                 ^^^^^ trailing `const` is only allowed in the HLSL dialect
                 ^^^^^ C++-style trailing `const` requires the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`).
    */
    {
        value = 0;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
    }
}

class Reference
{
    void method() const
    /*CHECK:
                  ^^^^^ trailing `const` is only allowed in the HLSL dialect
                  ^^^^^ C++-style trailing `const` requires the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`).
                  ^^^^^ receiver-mode specifier is not allowed on a class member
                  ^^^^^ receiver-mode specifier 'const' is not allowed on a class member; a class receiver always uses the `in` parameter-passing mode.
    */
    {}
}
