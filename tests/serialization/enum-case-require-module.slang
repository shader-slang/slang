//TEST_IGNORE_FILE:

public enum Mode
{
    [require(hlsl)] hlslMode = 0,
    [require(spirv)] spirvMode = 1,
}

public int readValue<let V : Mode>() { return int(V); }

public int readDefault<let V : Mode = Mode.hlslMode>() { return int(V); }

public struct Holder<let V : Mode>
{
    int value;
}

// The unused type parameter still introduces the selected default's requirements.
public int readTypeDefault<T = Holder<Mode.hlslMode>>() { return 1; }

public int readSymbolic<let tag : int>() { return int(Mode(tag + 1)); }

public int readSymbolicCancelled<let tag : int>() { return int(Mode(tag)) * 0; }
