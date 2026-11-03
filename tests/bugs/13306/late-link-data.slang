//TEST_IGNORE_FILE:

export struct Thing
{
    uint value;
};

export float4 readThing(Thing value)
{
    return float4(value.value);
}
