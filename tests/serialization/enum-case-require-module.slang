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

// Defaults must retain their source requirements without restricting callers that override them.
public int useDefault(Mode value = Mode.A) { return int(value); }
public int readDefault<let V : Mode = Mode.A>() { return 7; }
public int readFoldedDefault<let V : int = int(Mode.A) * 0>() { return V; }
public int readTypeDefault<T = Holder<int(Mode.A) * 0>>() { return 7; }

// This generic default is selected before serialization, inside an ordinary default expression.
public int useNestedDefault(int value = readFoldedDefault()) { return value; }

// The ordinary default must retain this explicit generic argument's source requirement too.
public int useExplicitGenericDefault(int value = readValue<Mode.A>()) { return value; }
