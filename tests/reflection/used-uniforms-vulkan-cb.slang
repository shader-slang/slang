// used-uniforms-vulkan-cb.slang

// Tests per uniform field tracking for a constant buffer whose parent
// binding lives in the DescriptorTableSlot category (a Vulkan style
// descriptor) rather than the ConstantBuffer category (a D3D b
// register). The explicit vk::binding gives it a non zero descriptor
// index. The emitter and the IR pass must agree on that parent key, so
// usedByteRanges has to attach to the param even though its binding is a
// descriptor slot and not a b register. The HLSL targeted cases
// (used-uniforms-single-cb etc.) only cover the ConstantBuffer path.

//TEST:REFLECTION:-stage compute -entry main -target spirv

struct Data
{
    float used0;
    float unused1;
    float used2;
    float unused3;
}

[[vk::binding(3, 0)]] ConstantBuffer<Data> c : register(b0);

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = c.used0 + c.used2;
}
