//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: a concrete type that
// other modules extend with a conformance, and an extern type that other
// modules define directly.

public interface IValue
{
    public uint value();
}

public struct Thing
{
}

public interface IRenderer
{
    public static uint value();
}

public extern struct Renderer : IRenderer;
public extern uint computeValue();

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
public void computeMain()
{
    output[0] = computeValue();
    output[1] = Renderer.value();
}
