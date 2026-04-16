// This is a cross-module test helper for cross-module-interface-var-default-initialization.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

#language 2025

public interface IHelper
{
    int getVal();
}

public struct HelperImpl1 : IHelper
{
    int storedVal;

    int getVal() { return storedVal; }
}

struct HelperImpl2 : IHelper
{
    int storedVal;

    int getVal() { return storedVal + 1; }
}

public struct StructWithIfaceField
{

    // This no longer compiles in Slang 2026, but this module uses 2025
    IHelper ifaceField = { };
//CHECK:                 ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior
//CHECK:                 ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized
}
