//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Semantic checking classifies an `inout` conversion as reading a property through its getter and
// writing the converted result back through its setter. Receiver validation must include both
// accessor operations.

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
    // also write the converted value back through `set`. Receiver validation rejects that setter
    // because the holder is immutable.
    mutateConvertedValue(holder.value);
//CHECK:                        ^^^^^ mutating method cannot be called on immutable value
//CHECK:                        ^^^^^ mutating method 'set' cannot be called on an immutable value
}

struct ProjectedValue
{
    int selected;
    int preserved;
}

struct SetterOnlyProjectedValueHolder
{
    property ProjectedValue value
    {
        set {}
    }
}

void initializeConvertedValue(out uint value)
{
    value = 1;
}

void rejectSetterOnlyProjectedConversion()
{
    SetterOnlyProjectedValueHolder holder;

    // The int-to-uint conversion replaces `selected` without reading its old value, but writing
    // that field back through the property must preserve `preserved`. The operation therefore
    // needs a getter or `ref` accessor for the complete property value.
    initializeConvertedValue(holder.value.selected);
//CHECK:                            ^^^^^ property or subscript cannot be read
//CHECK:                            ^^^^^ cannot read this property because it has no `get` or `ref` accessor
}

