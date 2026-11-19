// Ensure that InterlockedAdd on a typed buffer lowers to image atomics in SPIR-V,
// both for the direct path and for the GLSL -> glslang path.

//TEST:SIMPLE(filecheck=CHECK): -entry computeMain -stage compute -target spirv-assembly
//TEST:SIMPLE(filecheck=CHECK): -entry computeMain -stage compute -target spirv-assembly -emit-spirv-via-glsl

// CHECK: OpImageTexelPointer
// CHECK: OpAtomicIAdd

RWBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint originalValue;
    InterlockedAdd(outputBuffer[dispatchThreadID.x], dispatchThreadID.x, originalValue);
}
