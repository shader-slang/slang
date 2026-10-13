//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/hlsl-default-this-param-mode-dyn-interface.slang -std 2026 -no-codegen

// Non-experimental `dyn` dispatch cannot preserve mutations to an interface receiver. The writable
// HLSL method default, setter default, and explicit reference mode are therefore rejected. Getters
// are read-only by default, and explicit read-only receiver modes remain valid on every callable.

dyn interface ICounter
{
    void defaultWritable();
    /*CHECK:
         ^^^^^^^^^^^^^^^ dyn interfaces cannot have methods with writable receivers
         ^^^^^^^^^^^^^^^ `dyn` interface member 'defaultWritable' has a writable receiver; use `[nonmutating]` to make it read-only.
    */

    [__ref]
    void explicitReference();
    /*CHECK:
         ^^^^^^^^^^^^^^^^^ dyn interfaces cannot have methods with writable receivers
         ^^^^^^^^^^^^^^^^^ `dyn` interface member 'explicitReference' has a writable receiver; use `[nonmutating]` to make it read-only.
    */

    [nonmutating]
    void explicitNonmutating();

    [constref]
    void explicitBorrow();

    void trailingConst() const;

    void genericWritable<T>();
    /*CHECK:
         ^^^^^^^^^^^^^^^ dyn interfaces cannot have generic methods
         ^^^^^^^^^^^^^^^ dyn interfaces cannot have generic methods.
    */

    property int writableProperty
    {
        get;
        set;
        /*CHECK:
        ^^^ dyn interfaces cannot have methods with writable receivers
        ^^^ `dyn` interface member 'set' has a writable receiver; use `[nonmutating]` to make it read-only.
        */
    }

    property int explicitReadOnlyProperty
    {
        [nonmutating] get;
        [nonmutating] set;
    }

    __subscript(int index) -> int
    {
        get;
        set;
        /*CHECK:
        ^^^ dyn interfaces cannot have methods with writable receivers
        ^^^ `dyn` interface member 'set' has a writable receiver; use `[nonmutating]` to make it read-only.
        */
    }

    __subscript(uint index) -> int
    {
        [nonmutating] get;
        [nonmutating] set;
    }
}
