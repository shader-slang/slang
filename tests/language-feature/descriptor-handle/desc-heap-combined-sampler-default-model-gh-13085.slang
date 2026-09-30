// Positive companion to desc-heap-combined-sampler-gh-13085.slang for shader-slang/slang#13085.
// On the default SPIR-V and GLSL bindless model a combined texture-sampler is a single native
// descriptor loaded by one heap index (getDescriptorFromDynamicResourceHeap reads only `.x`), so
// building one from a single-index `ResourceDescriptorHeap[i]` handle must keep compiling -- the
// rejection is scoped to the targets that consume a separate sampler index. The `SPV`/`GLSL` checks
// below confirm the guard does not fire on the default model: the shader still compiles. They do not
// assert which heap slot is loaded. The SPIR-V run also passes `-warnings-as-errors all`, so the
// guards must stay warning-clean on plain SPIR-V -- in particular they must not force an implicit
// `spvDescriptorHeapEXT` capability upgrade (E41012). Direct `_Texture`, `.Handle`, and non-combined
// resource conversions are all covered here, WITHOUT spvDescriptorHeapEXT.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage fragment -entry main -profile spirv_1_6 -warnings-as-errors all -emit-spirv-directly
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main

[shader("fragment")]
float4 main(float3 coordinate : TEXCOORD0) : SV_Target
{
    Texture2D<float4> tex = ResourceDescriptorHeap[3];
    SamplerState samp = SamplerDescriptorHeap[4];

    Sampler3D volume = ResourceDescriptorHeap[11];

    Sampler3D.Handle handle = ResourceDescriptorHeap[12];
    Sampler3D volume2 = handle;

    //SPV: OpEntryPoint
    //GLSL: void main
    return tex.SampleLevel(samp, coordinate.xy, 0.0)
        + volume.SampleLevel(coordinate, 0.0)
        + volume2.SampleLevel(coordinate, 0.0);
}
