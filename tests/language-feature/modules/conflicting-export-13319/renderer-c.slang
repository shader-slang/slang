//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. A third module whose
// exports conflict with both renderer-a.slang and renderer-b.slang.

import contract;

public struct RendererC : IRenderer
{
    public static uint value() { return 33; }
}

export struct Renderer : IRenderer = RendererC;
export static const uint kCount = 303;
export uint pick() { return 333; }
