//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. Exports a second
// conformance of the `Thing` that extension-owner.slang defines.

import extension_owner;

public export extension Thing : IValue
{
    public uint value() { return 22; }
}
