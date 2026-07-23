// used-uniforms-vulkan-cb-nonzero-set.slang

// Regression test for per uniform field tracking on a constant buffer
// bound to a non zero Vulkan descriptor set. The sibling test
// used-uniforms-vulkan-cb.slang uses set 0, so the set never appears in
// the parent binding space and a pass that dropped the set would still
// match. Here the explicit vk::binding puts the CB in set 2, so the
// recorded parent binding space must include the set or the emitter's
// getBindingSpace lookup misses and usedByteRanges silently drops off
// the parameter. used0 and used2 are read; unused1 and unused3 are not.

//TEST:REFLECTION:-stage compute -entry main -target spirv

struct Data
{
    float used0;
    float unused1;
    float used2;
    float unused3;
}

[[vk::binding(3, 2)]] ConstantBuffer<Data> c;

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = c.used0 + c.used2;
}
