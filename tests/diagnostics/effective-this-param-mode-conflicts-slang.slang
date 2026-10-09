//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Invalid receiver-mode combinations recover to the dialect's default. A method declared on a
// value type in Slang uses `in` mode for `this` by default, so the body remains non-writable after
// the conflict is diagnosed.

struct Counter
{
    int value;

    [mutating]
    [constref]
    /*CHECK:
     ^^^^^^^^ duplicate modifier
     ^^^^^^^^ modifier 'constref' is redundant or conflicting with existing modifier 'mutating'
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

    [mutating]
    [__ref]
    /*CHECK:
     ^^^^^ duplicate modifier
     ^^^^^ modifier '__ref' is redundant or conflicting with existing modifier 'mutating'
    */
    void conflictingMutatingAndRef()
    {
        value++;
/*CHECK:
        ^^^^^ argument must be l-value
        ^^^^^ argument passed to parameter '0' must be l-value.
        ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
*/
    }
};
