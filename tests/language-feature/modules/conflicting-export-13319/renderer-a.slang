//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang.

import contract;

public struct RendererA : IRenderer
{
    public static uint value() { return 11; }
}

export struct Renderer : IRenderer = RendererA;
export static const uint kCount = 101;
export uint pick() { return 111; }
