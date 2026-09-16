// Positive companion to desc-heap-combined-sampler-gh-13085.slang for shader-slang/slang#13085.
// On the default SPIR-V and GLSL bindless model a combined texture-sampler is a single native
// descriptor loaded by one heap index (getDescriptorFromDynamicResourceHeap reads only `.x`), so
// building one from a single-index `ResourceDescriptorHeap[i]` handle is correct and must keep
// compiling -- the rejection is scoped to the targets that consume a separate sampler index. Both
// single-index paths (the direct `_Texture` conversion and the `.Handle` conversion) are covered
// here, WITHOUT spvDescriptorHeapEXT.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage fragment -entry main -profile spirv_1_6 -emit-spirv-directly
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main

[shader("fragment")]
float4 main(float3 coordinate : TEXCOORD0) : SV_Target
{
    Sampler3D volume = ResourceDescriptorHeap[11];

    Sampler3D.Handle handle = ResourceDescriptorHeap[12];
    Sampler3D volume2 = handle;

    //SPV: OpEntryPoint
    //GLSL: void main
    return volume.SampleLevel(coordinate, 0.0) + volume2.SampleLevel(coordinate, 0.0);
}
