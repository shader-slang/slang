//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

struct ConversionWritebackHolder
{
    int storage;

    property int value
    {
        [nonmutating]
        get { return storage; }

        [mutating]
        set { storage = newValue; }
    }
}

void mutateConvertedValue(inout uint value)
{
    value = 1;
}

void rejectMutatingConversionWriteback()
{
    let holder = ConversionWritebackHolder();

    // The int-to-uint conversion first reads the property through `get`, but the `inout` call must
    // also write the converted value back through `set`. The immutable holder rejects that setter.
    mutateConvertedValue(holder.value);
//CHECK:                        ^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                        ^^^^^ mutating 'set' accessor cannot be called on an immutable value
}
