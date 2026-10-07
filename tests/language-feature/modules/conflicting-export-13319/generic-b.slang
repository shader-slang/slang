//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. Both the generic
// link-time type and the generic `Box` conflict with generic-a.slang.

import generic_contract;

public struct GenericRendererB<T> : IRenderer
{
    public static uint value() { return 22; }
}

export struct GenericRenderer<T> : IRenderer = GenericRendererB<T>;

public export struct Box<T> : IValue
{
    public uint value() { return 44; }
}
