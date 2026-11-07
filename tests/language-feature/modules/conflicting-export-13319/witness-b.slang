//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. Both the conformance of
// `Thing` to `IValue` and the definition of `Renderer` conflict with
// witness-a.slang.

import witness_contract;

public export extension Thing : IValue
{
    public uint value() { return 22; }
}

public export struct Renderer : IRenderer
{
    public static uint value() { return 44; }
}
