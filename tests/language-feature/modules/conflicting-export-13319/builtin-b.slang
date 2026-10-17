//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. Both conformances
// conflict with builtin-a.slang.

import builtin_contract;

public export extension float : IValue
{
    public uint value() { return 22; }
}

public export extension<T> Box<T> : IValue
{
    public uint value() { return 44; }
}
