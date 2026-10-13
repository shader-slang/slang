//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A setter whose value type is non-copyable receives its new value through an implicit
// `BorrowIn` parameter. Semantic checking must therefore attempt to use the right-hand side's
// address before reading its value, matching the accessor selection used during lowering.

[__NonCopyableType]
struct NonCopyableValue
{
    int value;
}

static NonCopyableValue externalValue;

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

interface INonCopyableValueSink
{
    property item : NonCopyableValue { set; }
}

void rejectMutatingRef(inout NonCopyableValueSink destination)
{
    let source = MutatingRefSource();

    // The nonmutating getter can read the value, but semantic accessor checking selects the
    // address returned by `ref` because the setter passes its value as `BorrowIn`. The mutating
    // `ref` accessor cannot use the immutable `source` receiver.
    destination.item = source.item;
//CHECK:                      ^^^^ mutating method cannot be called on immutable value
//CHECK:                      ^^^^ mutating method 'ref' cannot be called on an immutable value
}

void rejectMutatingRefThroughExistential(inout INonCopyableValueSink destination)
{
    let source = MutatingRefSource();

    // Opening the existential destination introduces a local-binding sequence around the selected
    // setter. That prefix does not change how the setter's implicit `BorrowIn` value parameter
    // consumes the right-hand side.
    destination.item = source.item;
//CHECK:                      ^^^^ mutating method cannot be called on immutable value
//CHECK:                      ^^^^ mutating method 'ref' cannot be called on an immutable value
}

