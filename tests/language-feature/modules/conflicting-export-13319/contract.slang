//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: declares the link-time
// symbols that the renderer modules export.

public interface IRenderer
{
    public static uint value();
}

public extern struct Renderer : IRenderer;
public extern static const uint kCount;
public extern uint pick();
public extern static const uint kDefault = 5;

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
public void computeMain()
{
    output[0] = Renderer.value();
    output[1] = kCount;
    output[2] = pick();
    output[3] = kDefault;
}
