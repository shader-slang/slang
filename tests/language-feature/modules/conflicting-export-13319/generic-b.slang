//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. The generic link-time
// types and the generic `Box` all conflict with generic-a.slang.

import generic_contract;

public struct GenericRendererB<T> : IRenderer
{
    public static uint value() { return 22; }
}

export struct GenericRenderer<T> : IRenderer = GenericRendererB<T>;

public struct FixedRendererB : IRenderer
{
    public static uint value() { return 66; }
}

export struct FixedRenderer<T> : IRenderer = FixedRendererB;

public export struct Box<T> : IValue
{
    public uint value() { return 44; }
}
