//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: an interface that other
// modules conform builtin and generic types to.

public interface IValue
{
    public uint value();
}

public struct Box<T>
{
}

public extern uint computeValue();

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
public void computeMain()
{
    output[0] = computeValue();
}
