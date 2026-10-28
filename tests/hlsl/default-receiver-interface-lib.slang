// Non-test helper module for default-receiver-on-immutable-value.hlsl (no //TEST: directive).
// Its interface requirement has the Slang dialect's default `in` receiver, and its `[mutating]`
// method is called from HLSL code.
module default_receiver_interface_lib;

public interface IShape
{
    int area();
}

public struct SlangCounter
{
    public int value;

    [mutating]
    public int addAndGet(int delta)
    {
        value += delta;
        return value;
    }
}
