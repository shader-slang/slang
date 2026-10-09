//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/effective-this-param-mode-conflicts-hlsl.slang -no-codegen

// Invalid combinations and duplicate spellings are diagnosed, then recovery uses HLSL's default
// `inout` receiver, so each body remains writable.

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
    void duplicateAttribute()
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
