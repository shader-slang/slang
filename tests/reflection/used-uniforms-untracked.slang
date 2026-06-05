// used-uniforms-untracked.slang

// Tests the untracked-entry path of per uniform field tracking. A push
// constant constant buffer carries neither a ConstantBuffer (D3D b
// register) nor a DescriptorTableSlot (Vulkan descriptor) binding, only a
// PushConstantBuffer binding. The IR pass therefore has no parent binding
// identity to scope per byte ranges against, so it emits an isUntracked
// marker instead of a byte range list, and the emitter must surface that
// as "usedByteRangesUnavailable": true rather than a precise (and
// misleading) range set. Every other used-uniforms test exercises a
// tracked CB whose parent binding is known; this is the only one that
// reaches the untracked branch.

//TEST:REFLECTION:-stage compute -entry main -target spirv

struct PushData
{
    float used;
    float unused;
}

[[vk::push_constant]] ConstantBuffer<PushData> pc;

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = pc.used;
}
