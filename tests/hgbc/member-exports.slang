//TEST:SIMPLE(filecheck=CHECK): -target hgbc

public struct Counter
{
    uint value;

    [host]
    public __init(uint initialValue)
    {
        value = initialValue;
    }

    [host]
    public __init(bool enabled)
    {
        if (enabled)
            value = 1;
        else
            value = 0;
    }

    [host]
    [mutating]
    public void add(uint amount)
    {
        value += amount;
    }

    [host]
    [mutating]
    public void add(bool enabled)
    {
        if (enabled)
            value += 1;
    }

    [host]
    public uint getValue()
    {
        return value;
    }
}

// CHECK: OpExport Constructor
// CHECK: OpExport Constructor
// CHECK: OpExport Method
// CHECK: OpExport Method
// CHECK: OpExport Method
