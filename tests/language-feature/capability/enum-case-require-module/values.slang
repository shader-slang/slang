//TEST_IGNORE_FILE:

public enum TargetValue : uint
{
    [require(hlsl)]
    hlslOnly = 0,
    [require(spirv)]
    spirvOnly = 1,
    portable = 2,
}

public int consume(TargetValue value)
{
    return int(value);
}
