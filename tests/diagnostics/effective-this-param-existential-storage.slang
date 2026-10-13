//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// `maybeOpenExistential` must preserve the accessors of a property that returns an existential.
// Receiver validation must check the getter and any setter used to write the existential back.

[anyValueSize(16)]
interface IMutable
{
    [mutating] void mutate();
}

interface IRefReceiver
{
    [__ref] void mutateThroughReference();
}

struct MutableImpl : IMutable
{
    int value;
    [mutating] void mutate() { value++; }
}

struct Outer
{
    IMutable backing;

    property IMutable mutatingReadStorage
    {
        [mutating] get { return backing; }
        [nonmutating] set {}
    }

    property IMutable nonmutatingWritebackStorage
    {
        [nonmutating] get { return backing; }
        [nonmutating] set {}
    }
}

void testExistentialStorage(Outer input)
{
    let outer = input;

    // `maybeOpenExistential` reads the property before invoking the requirement, so this call
    // cannot use the mutating getter on the immutable receiver.
    outer.mutatingReadStorage.mutate();
/*CHECK:
          ^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
          ^^^^^^^^^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value
*/

    // The opened value is written back through the nonmutating getter/setter pair.
    outer.nonmutatingWritebackStorage.mutate();
}

void rejectRefReceiverOnInterfaceValue(IRefReceiver value)
{
    // Semantic checking opens the interface value to expose its concrete value. Lowering can copy
    // that value and write it back, but it cannot expose an address into the interface's payload
    // for a `ref` receiver.
    value.mutateThroughReference();
/*CHECK:
    ^^^^^ interface value does not provide an address
    ^^^^^ the concrete value stored in this interface value cannot be used where a direct address is required
*/
}

void acceptRefReceiverOnGenericValue<T : IRefReceiver>(inout T value)
{
    // Generic checking and lowering preserve the concrete value's storage through the constrained-
    // generic call, so its address is available.
    value.mutateThroughReference();
}

