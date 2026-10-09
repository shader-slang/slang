//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: a module that exports a
// link-time type and uses it, so that linking the source together with its
// precompiled form gives the linker two copies of one module.

public interface IRenderer
{
    public static uint value();
}

public struct RendererA : IRenderer
{
    public static uint value() { return 11; }
}

export struct Renderer : IRenderer = RendererA;

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = Renderer.value();
}
