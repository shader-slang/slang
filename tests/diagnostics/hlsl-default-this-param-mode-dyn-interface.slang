//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/hlsl-default-this-param-mode-dyn-interface.slang -std 2026 -no-codegen

// Non-experimental `dyn` dispatch cannot preserve mutations to an interface receiver. HLSL's
// default and every explicit receiver-mode spelling follow the same rule.

dyn interface ICounter
{
    void defaultWritable();
    /*CHECK:
         ^^^^^^^^^^^^^^^ dyn interfaces cannot have methods with writable receivers
         ^^^^^^^^^^^^^^^ `dyn` interface member 'defaultWritable' has a writable receiver; make it non-mutating with `[nonmutating]` (or trailing `const` in HLSL).
    */

    [mutating]
    void explicitMutating();
    /*CHECK:
         ^^^^^^^^^^^^^^^^ dyn interfaces cannot have methods with writable receivers
         ^^^^^^^^^^^^^^^^ `dyn` interface member 'explicitMutating' has a writable receiver; make it non-mutating with `[nonmutating]` (or trailing `const` in HLSL).
    */

    [__ref]
    void explicitReference();
    /*CHECK:
         ^^^^^^^^^^^^^^^^^ dyn interfaces cannot have methods with writable receivers
         ^^^^^^^^^^^^^^^^^ `dyn` interface member 'explicitReference' has a writable receiver; make it non-mutating with `[nonmutating]` (or trailing `const` in HLSL).
    */

    [nonmutating]
    void explicitNonmutating();

    [constref]
    void explicitBorrow();

    void trailingConst() const;
}
