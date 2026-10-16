//TEST:SIMPLE: -target spirv -no-codegen -warnings-as-errors all

// Check that a read in a constant-false branch is removed before diagnostics.
float unreachableRead()
{
    float value;
    if (false)
        return value;
    return 0;
}

// Check that an unused computation does not keep an uninitialized read alive.
float unusedRead()
{
    float value;
    float unusedResult = value + 1;
    return 0;
}

// Check that an unused partial-vector read does not introduce a warning.
float unusedPartialRead(bool condition)
{
    float4 value;
    if (condition) value.rgb = float3(1, 2, 3);
    float unusedResult = value.a;
    return 0;
}

[shader("compute")]
void main() {}
