//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. Every export here
// conflicts with the one in renderer-a.slang.

import contract;

public struct RendererB : IRenderer
{
    public static uint value() { return 22; }
}

export struct Renderer : IRenderer = RendererB;
export static const uint kCount = 202;
export uint pick() { return 222; }
