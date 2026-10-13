//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Error recovery uses Slang's default `in` receiver after a repeated or conflicting mode
// specifier. The checker therefore also diagnoses the attempted field mutations below.

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
        /*CHECK:
        ^^^^^ argument must be l-value
        ^^^^^ argument passed to parameter '0' must be l-value.
        ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        */
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
        /*CHECK:
        ^^^^^ argument must be l-value
        ^^^^^ argument passed to parameter '0' must be l-value.
        ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        */
    }
};
