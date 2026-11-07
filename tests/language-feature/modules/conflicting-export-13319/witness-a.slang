//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang.

import witness_contract;

public export extension Thing : IValue
{
    public uint value() { return 11; }
}

uint useValue<T : IValue>(T t) { return t.value(); }

export uint computeValue()
{
    Thing t;
    return useValue(t);
}

public export struct Renderer : IRenderer
{
    public static uint value() { return 33; }
}
