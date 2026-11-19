//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: exports a type together
// with its conformance, which extension-other.slang also exports.

public interface IValue
{
    public uint value();
}

public export struct Thing
{
}

public export extension Thing : IValue
{
    public uint value() { return 11; }
}

uint useValue<T : IValue>(T t) { return t.value(); }

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
public void computeMain()
{
    Thing t;
    output[0] = useValue(t);
}
