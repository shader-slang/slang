//TEST_IGNORE_FILE:

public enum Mode
{
    [require(hlsl)] A = 0,
    [require(spirv)] B = 1,
}

public int readValue<let V : Mode>() { return 7; }

public struct Holder<let V : int>
{
    public int payload;
}

// Both the inferred requirement and the case's explicit requirement must survive serialization.
public typealias HlslAlias = Holder<int(Mode.A) * 0>;

public int fromTag<let N : int>() { return int(Mode(N)); }
