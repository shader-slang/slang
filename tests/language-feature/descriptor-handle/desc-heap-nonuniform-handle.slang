// The `.Handle` construction spelling of a non-uniform descriptor-heap index: a `Texture2D.Handle`
// built from `uint2(NonUniformResourceIndex(tid.x), 0)` is implicitly converted back to
// `Texture2D`, and the peephole fold that collapses this uint2-to-handle-to-uint2 round-trip must
// keep the non-uniform marker on the emerging scalar heap index — the emitted code must mark
// the heap access non-uniform the same way the direct subscript spelling does (see
// `desc-heap-nonuniform.slang`), instead of leaving the marker nested inside the `uint2` /
// `uvec2` construction. The resource and the sampler heap are both covered, because the two heap
// types load through distinct intrinsics (`__loadResourceDescriptorFromHeap` and
// `__loadSamplerDescriptorFromHeap`), so a marker-dropping regression in one chain must not be
// masked by the other still passing. SPIR-V decoration for the resource heap is covered by
// `desc-heap-nonuniform-spirv.slang`.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -entry computeMain -emit-spirv-via-glsl

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), 0) };
    Texture2D tex = th;
    SamplerState.Handle sh = { uint2(NonUniformResourceIndex(tid.y), 0) };
    SamplerState s = sh;
    outputBuffer[0] = tex.SampleLevel(s, float2(0.0, 0.0), 0.0).x;
}

// Both heap indices must be wrapped in `NonUniformResourceIndex` directly inside the subscript
// brackets in the emitted code, not nested inside the `uint2` construction the source shows.
// HLSL: ResourceDescriptorHeap[NonUniformResourceIndex(
// HLSL: SamplerDescriptorHeap[NonUniformResourceIndex(

// DXIL asserts the same propagation property as `desc-heap-nonuniform.slang`, whose DXIL check
// block explains the `createHandleFromHeap` operand layout these anchors pin.
// DXIL: define void @computeMain(
// DXIL-DAG: @dx.op.createHandleFromHeap(i32 218, i32 %{{[0-9]+}}, i1 false, i1 true)
// DXIL-DAG: @dx.op.createHandleFromHeap(i32 218, i32 %{{[0-9]+}}, i1 true, i1 true)

// GLSL asserts the same property as `desc-heap-nonuniform.slang`, whose check block explains the
// combined `sampler2D(texture, sampler)` constructor shape these anchors pin and the heap-name
// suffix wildcards.
// GLSL: #extension GL_EXT_nonuniform_qualifier : require
// GLSL: sampler2D(_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(
// GLSL: ,_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(

// Routing the same shader through glslang confirms the downstream GLSL compiler keeps the
// non-uniform promise: the emitted SPIR-V decorates both heap access chains NonUniform.
// GLSLSPV: OpCapability ShaderNonUniform
// GLSLSPV-DAG: OpAccessChain %{{.*}} %_slang_resource_heap{{[0-9_]*}} %{{.*}}NonUniform
// GLSLSPV-DAG: OpAccessChain %{{.*}} %_slang_resource_heap{{[0-9_]*}} %{{.*}}NonUniform
