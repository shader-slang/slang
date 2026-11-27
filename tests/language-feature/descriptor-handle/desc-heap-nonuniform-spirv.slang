// Verify that a descriptor-heap index wrapped in `NonUniformResourceIndex` keeps its `NonUniform`
// decoration through SPIR-V's default (runtime-array) heap lowering.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=SPVHANDLE): -target spirv-asm -stage compute -entry mainHandle
//TEST:SIMPLE(filecheck=SPVNONCONST): -target spirv-asm -stage compute -entry mainNonConstSibling
//TEST:SIMPLE(filecheck=SPVUNIFORM): -target spirv-asm -stage compute -entry mainUniform

// With `spvDescriptorHeapEXT`, heap access is non-uniform by default, so no decoration is expected.
//TEST:SIMPLE(filecheck=SPVCAP): -target spirv-asm -stage compute -entry main -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=SPVCAPHANDLE): -target spirv-asm -stage compute -entry mainHandle -capability spvDescriptorHeapEXT

RWStructuredBuffer<float> outputBuffer;
SamplerState samp;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Texture2D tex = ResourceDescriptorHeap[NonUniformResourceIndex(tid.x)];
    SamplerState s = SamplerDescriptorHeap[NonUniformResourceIndex(tid.y)];
    outputBuffer[0] = tex.SampleLevel(s, float2(0, 0), 0).x;
}

// The two heaps are not emitted in source order, so either may get the `_0` suffix.
// SPV-DAG: OpCapability ShaderNonUniform
// SPV-DAG: OpCapability SampledImageArrayNonUniformIndexing
// SPV-DAG: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}}NonUniform
// SPV-DAG: OpAccessChain %{{.*}} %__slang_resource_heap_0 %{{.*}}NonUniform

// SPVCAP-NOT: NonUniform
// SPVCAP: OpCapability DescriptorHeapEXT
// SPVCAP-NOT: NonUniform
// SPVCAP: OpFunctionEnd

[shader("compute")]
[numthreads(1, 1, 1)]
void mainHandle(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), 0) };
    Texture2D tex = th;
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// SPVHANDLE-DAG: OpCapability ShaderNonUniform
// SPVHANDLE-DAG: OpCapability SampledImageArrayNonUniformIndexing
// SPVHANDLE-DAG: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}}NonUniform

// SPVCAPHANDLE-NOT: NonUniform
// SPVCAPHANDLE: OpCapability DescriptorHeapEXT
// SPVCAPHANDLE-NOT: NonUniform
// SPVCAPHANDLE: OpFunctionEnd

// Only the first lane of the handle indexes the heap, so the decoration must follow that lane.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainNonConstSibling(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), tid.y) };
    Texture2D tex = th;
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// SPVNONCONST-DAG: OpCapability ShaderNonUniform
// SPVNONCONST-DAG: %[[IDX:[a-zA-Z0-9_]+]] = OpCompositeExtract %uint %{{.*}} 0 {{.*}}NonUniform
// SPVNONCONST-DAG: OpAccessChain %{{.*}} %__slang_resource_heap %[[IDX]] {{.*}}NonUniform

[shader("compute")]
[numthreads(1, 1, 1)]
void mainUniform(uint3 tid : SV_DispatchThreadID)
{
    Texture2D tex = ResourceDescriptorHeap[0];
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// SPVUNIFORM-NOT: NonUniform
// SPVUNIFORM: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}}
// SPVUNIFORM-NOT: NonUniform
// SPVUNIFORM: OpFunctionEnd
