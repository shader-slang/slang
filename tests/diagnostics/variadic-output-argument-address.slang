// DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void writePack<each T>(out expand each T values) {}

struct Properties<each T>
{
    Tuple<expand each T> storage;
    property Tuple<expand each T> values
    {
        get
        {
            return storage;
        }
        set
        {
            storage = newValue;
        }
    }
}

void writeProperties<each T>(inout Properties<expand each T> properties)
{
    writePack(expand each properties.values);
    // CHECK: expanded output arguments must name addressable storage
}
