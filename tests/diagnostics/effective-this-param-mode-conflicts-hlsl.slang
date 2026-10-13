//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/effective-this-param-mode-conflicts-hlsl.slang -no-codegen

// Semantic checking diagnoses every repeated or conflicting receiver-mode spelling after the
// first. Error recovery uses HLSL's default `inout` receiver, so the method bodies remain writable.

struct Counter
{
    int value;

    [mutating]
    [nonmutating]
    /*CHECK:
     ^^^^^^^^^^^ conflicting receiver-mode specifiers
     ^^^^^^^^^^^ receiver-mode specifier 'nonmutating' conflicts with earlier specifier 'mutating'
    */
    void conflictingAttributes()
    {
        value++;
    }

    [constref]
    [__ref]
    /*CHECK:
     ^^^^^ conflicting receiver-mode specifiers
     ^^^^^ receiver-mode specifier '__ref' conflicts with earlier specifier 'constref'
    */
    void conflictingConstRefAndRef()
    {
        // Because the explicit modes conflict, recovery must use HLSL's `inout` default. This
        // write would be rejected if recovery accidentally selected `constref`.
        value++;
    }

    [nonmutating]
    [nonmutating]
    /*CHECK:
     ^^^^^^^^^^^ duplicate modifier
     ^^^^^^^^^^^ modifier 'nonmutating' is redundant or conflicting with existing modifier 'nonmutating'
    */
    void repeatedAttribute()
    {
        value++;
    }

    [mutating]
    void attributeAndTrailingConst() const
    /*CHECK:
                                     ^^^^^ conflicting receiver-mode specifiers
                                     ^^^^^ receiver-mode specifier 'const' conflicts with earlier specifier 'mutating'
    */
    {
        value++;
    }

    [mutating]
    void genericAttributeAndTrailingConst<T>() const
    /*CHECK:
                                               ^^^^^ conflicting receiver-mode specifiers
                                               ^^^^^ receiver-mode specifier 'const' conflicts with earlier specifier 'mutating'
    */
    {
        // Error recovery must find the HLSL default through the method's generic wrapper, so this
        // write remains valid after the conflicting explicit spellings are rejected.
        value++;
    }

    // Semantically equivalent spellings are still two receiver-mode specifiers.
    [nonmutating]
    void conflictingEquivalentSpellings() const
    /*CHECK:
                                          ^^^^^ conflicting receiver-mode specifiers
                                          ^^^^^ receiver-mode specifier 'const' conflicts with earlier specifier 'nonmutating'
    */
    {
        value++;
    }

    void repeatedTrailingConst() const const
    /*CHECK:
                                       ^^^^^ duplicate modifier
                                       ^^^^^ modifier 'const' is redundant or conflicting with existing modifier 'const'
    */
    {
        value++;
    }
};
