//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// An initializer with a `__ref` parameter requires its argument's address. A property can supply
// that address only through a `ref` accessor, not through a getter/setter pair.

struct ConvertedValue
{
    int value;

    __implicit_conversion(0)
    __init(__ref int source)
    {
        value = source;
    }
}

struct GetterSetterStorage
{
    int backing;

    property int value
    {
        get { return backing; }
        set { backing = newValue; }
    }
}

ConvertedValue rejectConversionWithoutRefAccessor(GetterSetterStorage storage)
{
    // The selected initializer requires the address of its argument. A getter and setter can
    // support read-modify-write, but only a `ref` accessor can provide that address.
    return storage.value;
//CHECK:           ^^^^^ property or subscript does not provide an address
//CHECK:           ^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
}

static int referencedValue;

struct RefStorage
{
    property int value
    {
        [nonmutating]
        ref { return referencedValue; }
    }
}

ConvertedValue acceptConversionWithRefAccessor(RefStorage storage)
{
    // The `ref` accessor provides the direct address required by the selected initializer.
    return storage.value;
}

