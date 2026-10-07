//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang.

import builtin_contract;

public export extension float : IValue
{
    public uint value() { return 11; }
}

public export extension<T> Box<T> : IValue
{
    public uint value() { return 33; }
}

uint useValue<T : IValue>(T t) { return t.value(); }

export uint computeValue()
{
    Box<int> box;
    return useValue(1.0f) + useValue(box);
}
