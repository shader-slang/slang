//TEST_IGNORE_FILE:

export struct Thing
{
    Texture2D<float4> texture;
    uint value;
};

export float4 readThing(Thing value)
{
    return value.texture.Load(int3(0)) + float4(value.value);
}
