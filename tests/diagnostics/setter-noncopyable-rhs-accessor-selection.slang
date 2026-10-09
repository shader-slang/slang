//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A setter whose value type is non-copyable receives its new value through an implicit
// `BorrowIn` parameter. Semantic checking must therefore attempt to use the right-hand side's
// address before reading its value, matching the accessor selection used during lowering.

[__NonCopyableType]
struct NonCopyableValue
{
    int value;
}

NonCopyableValue externalValue;

struct MutatingRefSource
{
    property NonCopyableValue item
    {
        [nonmutating]
        get
        {
            NonCopyableValue result;
            result.value = 0;
            return result;
        }

        [mutating]
        ref { return externalValue; }
    }
}

struct NonCopyableValueSink
{
    property NonCopyableValue item
    {
        set(NonCopyableValue newValue) {}
    }
}

void rejectMutatingRef(inout NonCopyableValueSink destination)
{
    let source = MutatingRefSource();

    // The nonmutating getter can read the value, but the setter's `BorrowIn` parameter instead
    // selects the address returned by `ref`. The mutating `ref` accessor cannot use the immutable
    // `source` receiver.
    destination.item = source.item;
//CHECK:                      ^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                      ^^^^ mutating 'ref' accessor cannot be called on an immutable value
}
