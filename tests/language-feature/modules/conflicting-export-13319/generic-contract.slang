//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: a generic link-time type
// and an interface for generic types that other modules export.

public interface IValue
{
    public uint value();
}

public interface IRenderer
{
    public static uint value();
}

public extern struct GenericRenderer<T> : IRenderer;
public extern uint computeValue();

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
public void computeMain()
{
    output[0] = GenericRenderer<int>.value();
    output[1] = computeValue();
}
