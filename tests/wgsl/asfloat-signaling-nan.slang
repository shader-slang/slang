//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

// When `asfloat` of a constant is folded, the f32 value is held as a double, and widening a
// signaling NaN quiets it. On WGSL, where `asfloat` is folded, `asfloat(0x7F800001u)` therefore
// reaches the emitter as the quiet NaN 0x7FC00001. Targets that keep `asfloat` as an opaque
// intrinsic keep the signaling bits. This test pins the WGSL behavior, so a change to it is
// deliberate; it has no run-time row because the Vulkan readback keeps 0x7F800001.

// WGSL: fn computeMain
// WGSL: _slang_getNan(0x7FC00001u)

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = asuint(asfloat(0x7F800001u));
}
