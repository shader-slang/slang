// A descriptor-heap `ConstantBuffer<T>` whose element uses 16-bit storage must declare the
// matching 8/16-bit storage capability. The untyped Uniform buffer pointer accesses the
// pointee just like a typed pointer would, so it needs the same capability a typed pointer
// to that type requires.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct SmallData
{
    half a;
    uint16_t b;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    ConstantBuffer<SmallData> uniforms = ResourceDescriptorHeap[0];
    RWStructuredBuffer<float> output = ResourceDescriptorHeap[1];
    output[tid.x] = float(uniforms.a) + float(uniforms.b);
}

// The 16-bit uniform-storage capability must be declared for the untyped Uniform access.
// CHECK-DAG: OpCapability UniformAndStorageBuffer16BitAccess
// CHECK-DAG: OpTypeBufferEXT Uniform
// CHECK-DAG: OpTypeUntypedPointerKHR Uniform{{$}}
