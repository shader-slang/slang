//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Error recovery uses Slang's default `in` receiver after a repeated or conflicting mode
// specifier. The attempted field mutations below therefore also diagnose.

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
