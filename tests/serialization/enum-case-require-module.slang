//TEST_IGNORE_FILE:

public enum Mode
{
    [require(hlsl)] hlslMode = 0,
    [require(spirv)] spirvMode = 1,
}

public int readValue<let V : Mode>() { return int(V); }

public int readDefault<let V : Mode = Mode.hlslMode>() { return int(V); }
