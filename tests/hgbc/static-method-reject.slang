//TEST:SIMPLE(filecheck=CHECK): -target hgbc

public struct Counter
{
    [host]
    public static uint clear(uint value)
    {
        return value;
    }
}

// CHECK: OpExport StaticMethod
