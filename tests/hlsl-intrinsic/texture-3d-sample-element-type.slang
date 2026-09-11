// CUDA's texture-fetch template family instantiates only scalar / 2- / 4-component
// result types, so a 3-component texel type fetches four components and reshapes the
// result back to the declared width (shader-slang/slang#12634).
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

Texture3D<float> texF1;
Texture3D<float2> texF2;
Texture3D<float3> texF3;
Texture3D<float4> texF4;
Texture3D<int3> texI3;
Texture3D<uint3> texU3;
Texture2DArray<float3> texArrF3;
Texture1D<float3> tex1dF3;
TextureCube<float3> texCubeF3;
SamplerState samp;

// Combined texture-samplers exercise the other CUDA arm (texture at operand $0,
// coordinate at operand $1 with no separate sampler operand).
Sampler3D<float3> combF3;
Sampler2DArray<float3> combArrF3;

RWStructuredBuffer<float> outF1;
RWStructuredBuffer<float2> outF2;
RWStructuredBuffer<float3> outF3;
RWStructuredBuffer<float4> outF4;
RWStructuredBuffer<int3> outI3;
RWStructuredBuffer<uint3> outU3;

[numthreads(1, 1, 1)]
void computeMain()
{
    // A 3-component texel type fetches four components and reshapes to float3; the
    // 3-component template argument is never instantiated. Each negative check
    // precedes its positive so the exclusion window ends at the correct fetch.
    // CUDA-NOT: tex3DLod<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex3DLod<float4>
    outF3[0] = texF3.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);

    // CUDA-NOT: tex3D<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex3D<float4>
    outF3[1] = texF3.Sample(samp, float3(0.5, 0.5, 0.5));

    // CUDA-NOT: tex2DLayeredLod<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex2DLayeredLod<float4>
    outF3[2] = texArrF3.SampleLevel(samp, float3(0.5, 0.5, 0.0), 0.0);

    // Combined texture-samplers place the coordinate at operand $1, not $2, and
    // widen/narrow identically.
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex3DLod<float4>
    outF3[3] = combF3.SampleLevel(float3(0.5, 0.5, 0.5), 0.0);

    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex3D<float4>
    outF3[4] = combF3.Sample(float3(0.5, 0.5, 0.5));

    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex2DLayeredLod<float4>
    outF3[5] = combArrF3.SampleLevel(float3(0.5, 0.5, 0.0), 0.0);

    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex1DLod<float4>
    outF3[6] = tex1dF3.SampleLevel(samp, 0.5, 0.0);

    // CUDA: _slang_vector_reshape<float, 3, float, 4>(texCubemapLod<float4>
    outF3[7] = texCubeF3.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);

    // Integer texel types widen and reshape to int4/uint4, not int3/uint3.
    // CUDA-NOT: tex3DLod<int3
    // CUDA: _slang_vector_reshape<int, 3, int, 4>(tex3DLod<int4>
    outI3[0] = texI3.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);

    // CUDA-NOT: tex3DLod<uint3
    // CUDA: _slang_vector_reshape<uint, 3, uint, 4>(tex3DLod<uint4>
    outU3[0] = texU3.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);

    // Scalar / 2- / 4-component widths fetch their element type directly, with no
    // reshape (the narrowing is identity for them).
    // CUDA-NOT: _slang_vector_reshape<float, 1
    // CUDA: tex3DLod<float>
    outF1[0] = texF1.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);

    // CUDA-NOT: _slang_vector_reshape<float, 2
    // CUDA: tex3DLod<float2 >
    outF2[0] = texF2.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);

    // CUDA-NOT: _slang_vector_reshape<float, 4
    // CUDA: tex3DLod<float4 >
    outF4[0] = texF4.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);
}

// The PTX target compiles through NVRTC and fetches four components per 3D sample
// (signed and unsigned integer fetches keep their signedness).
// PTX: tex.level.3d.v4.f32
// PTX: tex.3d.v4.f32
// PTX: tex.level.3d.v4.s32
// PTX: tex.level.3d.v4.u32
