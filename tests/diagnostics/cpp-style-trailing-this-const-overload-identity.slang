//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-overload-identity.slang -no-codegen

// Receiver presence and parameter-passing mode do not participate in overload identity.
// Declarations with different effective modes or different receiver presence therefore conflict
// within one redeclaration family. Two spellings of the same effective mode remain compatible and
// resolve as one callable.

struct Value
{
    // HLSL's fallback and trailing `const` select different receiver modes.
    void modeMismatch();
    /*CHECK:
         ^^^^^^^^^^^^ see previous declaration of 'modeMismatch'
    */

    void modeMismatch() const;
    /*CHECK:
         ^^^^^^^^^^^^ function receiver mismatch
         ^^^^^^^^^^^^ function 'modeMismatch' has receiver type 'Value' with parameter-passing mode 'in', but its previous declaration has receiver type 'Value' with parameter-passing mode 'inout'
    */

    // Static and instance declarations disagree on whether the function has a receiver.
    static void receiverPresenceMismatch();
    /*CHECK:
                ^^^^^^^^^^^^^^^^^^^^^^^^ see previous declaration of 'receiverPresenceMismatch'
    */

    void receiverPresenceMismatch();
    /*CHECK:
         ^^^^^^^^^^^^^^^^^^^^^^^^ function receiver mismatch
         ^^^^^^^^^^^^^^^^^^^^^^^^ function 'receiverPresenceMismatch' and its previous declaration disagree on whether they have an instance receiver
    */

    // `[nonmutating]` and trailing `const` both select the `in` receiver mode.
    [nonmutating]
    void equivalentMode();

    void equivalentMode() const;

    void useEquivalentMode()
    {
        equivalentMode();
    }
};
