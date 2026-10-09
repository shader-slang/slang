//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/effective-this-param-mode-conflicts-hlsl.slang -no-codegen

// Repeated or conflicting receiver-mode specifiers diagnose each spelling after the first. Error
// recovery uses HLSL's default `inout` receiver, so the method bodies remain writable.

struct Counter
{
    int value;

    [mutating]
    [nonmutating]
    /*CHECK:
     ^^^^^^^^^^^ duplicate modifier
     ^^^^^^^^^^^ modifier 'nonmutating' is redundant or conflicting with existing modifier 'mutating'
    */
    void conflictingAttributes()
    {
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
                                     ^^^^^ duplicate modifier
                                     ^^^^^ modifier 'const' is redundant or conflicting with existing modifier 'mutating'
    */
    {
        value++;
    }
};
