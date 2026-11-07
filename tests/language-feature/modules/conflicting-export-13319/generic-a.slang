//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang.

import generic_contract;

public struct GenericRendererA<T> : IRenderer
{
    public static uint value() { return 11; }
}

export struct GenericRenderer<T> : IRenderer = GenericRendererA<T>;

public struct FixedRendererA : IRenderer
{
    public static uint value() { return 55; }
}

export struct FixedRenderer<T> : IRenderer = FixedRendererA;

public export struct Box<T> : IValue
{
    public uint value() { return 33; }
}

uint useValue<T : IValue>(T t) { return t.value(); }

export uint computeValue()
{
    Box<int> box;
    return useValue(box);
}
