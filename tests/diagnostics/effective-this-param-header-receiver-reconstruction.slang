//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SLANG): tests/diagnostics/effective-this-param-header-receiver-reconstruction.slang -DTEST_CASE=1 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=HLSL): -lang hlsl tests/diagnostics/effective-this-param-header-receiver-reconstruction.slang -DTEST_CASE=2 -no-codegen

// A default argument is checked while its method signature is still in progress. Lookup records a
// `ThisValue` breadcrumb and its originating scope without asking how the receiver is passed.
// Expression reconstruction replays that selected path and reads the receiver mode that signature
// checking published before checking the header; it does not recursively restart declaration
// checking.

#if TEST_CASE == 1
struct SlangHeaderLookup
{
    [mutating]
    int mutate()
    {
        return 1;
    }

    [nonmutating]
    void read(int replacement = mutate())
//SLANG:                              ^ mutating method cannot be called on immutable value
//SLANG:                              ^ mutating method 'mutate' cannot be called on an immutable value
//SLANG:                        ^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    {}
};
#endif

#if TEST_CASE == 2
struct HLSLHeaderLookup
{
    int mutate()
    {
        return 1;
    }

    void read(int replacement = mutate()) const
//HLSL:                               ^ mutating method cannot be called on immutable value
//HLSL:                               ^ mutating method 'mutate' cannot be called on an immutable value
//HLSL:                         ^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    {}
};
#endif
