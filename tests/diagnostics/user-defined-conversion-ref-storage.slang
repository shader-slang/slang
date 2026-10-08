//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

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
//CHECK:           ^^^^^ argument must be l-value
//CHECK:           ^^^^^ argument passed to parameter '0' must be l-value.
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
