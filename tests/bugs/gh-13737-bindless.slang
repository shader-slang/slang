//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=NOHEAPSAMPLER): -target hlsl -entry main -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry wgslMain -stage compute

// shader-slang/slang#13737: `Load`, the subscript and `GetDimensions` on a bindless combined
// texture-sampler handle use only the texture. On HLSL the handle resolves to a texture taken
// from `ResourceDescriptorHeap`, and the call goes to that texture.

uniform StructuredBuffer<Sampler2D<float4>.Handle> handles;
RWStructuredBuffer<float4> outBuf;

// HLSL: Texture2D<float4 > [[T0:_S[0-9]+]] = Texture2D<float4 >(ResourceDescriptorHeap[handles_{{[0-9]+}}.Load(NonUniformResourceIndex(
// HLSL: [[T0]].GetDimensions(w_{{[0-9]+}}, h_{{[0-9]+}});
// HLSL: Texture2D<float4 > [[T1:_S[0-9]+]] = Texture2D<float4 >(ResourceDescriptorHeap[
// HLSL: [[T1]].Load(int3(
// HLSL: Texture2D<float4 > [[T2:_S[0-9]+]] = Texture2D<float4 >(ResourceDescriptorHeap[
// HLSL: [[T2]].Load(int3(uint3(

// A bindless combined handle's sampler would come from `SamplerDescriptorHeap`; none of these
// operations may read it.
// NOHEAPSAMPLER-NOT: SamplerDescriptorHeap

// DXIL: call %dx.types.Dimensions @dx.op.getDimensions
// DXIL: call %dx.types.ResRet.f32 @dx.op.textureLoad

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    uint i = id.x;
    uint w, h;
    handles[NonUniformResourceIndex(i)].GetDimensions(w, h);
    outBuf[0] = handles[NonUniformResourceIndex(i)].Load(int3(int2(id.xy), 0))
              + handles[NonUniformResourceIndex(i)][id.xy] + float4(w, h, 0, 0);
}

// WGSL keeps the `$q` string for `Load`, so the bindless texture is read directly.
// WGSL: textureLoad((_slang_resource_heap_{{[0-9]+}}[
[shader("compute")]
[numthreads(1, 1, 1)]
void wgslMain(uint3 id: SV_DispatchThreadID)
{
    outBuf[0] = handles[id.x].Load(int3(int2(id.xy), 0));
}
