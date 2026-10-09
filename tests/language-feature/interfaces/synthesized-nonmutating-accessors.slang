//TEST:SIMPLE: -no-codegen

// Differing value and index types force property and subscript wrapper synthesis. Both concrete
// setters are genuinely nonmutating, so this test prevents the invalid-receiver diagnostics from
// being implemented by rejecting every synthesized `[nonmutating]` setter.

interface INonmutatingProperty
{
    property int value
    {
        get;
        [nonmutating] set;
    }
}

struct ValidProperty : INonmutatingProperty
{
    property uint value
    {
        get { return 0; }
        [nonmutating] set {}
    }
}

interface INonmutatingSubscript
{
    __subscript(int index) -> int
    {
        get;
        [nonmutating] set;
    }
}

struct ValidSubscript : INonmutatingSubscript
{
    RWStructuredBuffer<int> data;

    __subscript(uint index) -> uint
    {
        get { return data[index]; }
        [nonmutating] set { data[index] = newValue; }
    }
}
