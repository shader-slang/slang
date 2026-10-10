// SampleGrad lowers to the CUDA `tex*Grad<T>` runtime built-ins for every texture shape
// (shader-slang/slang#13556). The built-ins take float4 gradients for 3D and cube shapes,
// so float3 gradients widen; 3-component texel types fetch four components and reshape.
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

Texture1D<float4> t1D;
Texture1DArray<float4> t1DArr;
Texture2D<float4> t2D;
Texture2DArray<float4> t2DArr;
Texture3D<float4> t3D;
TextureCube<float4> tCube;
TextureCubeArray<float4> tCubeArr;

Texture1D<float3> t1DF3;
Texture1DArray<float3> t1DArrF3;
Texture2D<float3> t2DF3;
Texture2DArray<float3> t2DArrF3;
TextureCube<float3> tCubeF3;
Texture3D<float3> t3DF3;
TextureCubeArray<float3> tCubeArrF3;
Texture2D<int3> t2DI3;
Texture2D<uint3> t2DU3;
Texture2D<float> t2DF1;
Texture2D<float2> t2DF2;
SamplerState samp;

Sampler1D<float4> c1D;
Sampler1DArray<float4> c1DArr;
Sampler2D<float4> c2D;
Sampler2DArray<float4> c2DArr;
Sampler3D<float4> c3D;
SamplerCube<float4> cCube;
SamplerCubeArray<float4> cCubeArr;
Sampler3D<float3> c3DF3;

RWStructuredBuffer<float> outF1;
RWStructuredBuffer<float2> outF2;
RWStructuredBuffer<float3> outF3;
RWStructuredBuffer<float4> outF4;
RWStructuredBuffer<int3> outI3;
RWStructuredBuffer<uint3> outU3;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Each float4 call below pins its full argument list: the coordinates, then the int layer
    // for array shapes, then gradX before gradY. The emitter hoists the float2/float3 gradient
    // constants into temporaries that later calls share, so the checks capture those names
    // from their declarations; the 1D gradients stay inline literals.
    // CUDA: tex1DGrad<float4 >({{.*}}t1D_0), ((0.5f)), ((0.25f)), ((0.125f)))
    outF4[0] = t1D.SampleGrad(samp, 0.5, 0.25, 0.125);

    // CUDA: tex1DLayeredGrad<float4 >({{.*}}t1DArr_0), ({{.*}}).x, int(({{.*}}).y), ((0.25f)), ((0.125f)))
    outF4[1] = t1DArr.SampleGrad(samp, float2(0.5, 1.0), 0.25, 0.125);

    // CUDA: [[GX2:_S[0-9]+]] = make_float2{{ *}}(0.25f, 0.0f);
    // CUDA: [[GY2:_S[0-9]+]] = make_float2{{ *}}(0.0f, 0.125f);
    // CUDA: tex2DGrad<float4 >({{.*}}t2D_0), ({{.*}}).x, ({{.*}}).y, (([[GX2]])), (([[GY2]])))
    outF4[2] = t2D.SampleGrad(samp, float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA: tex2DLayeredGrad<float4 >({{.*}}t2DArr_0), ({{.*}}).x, ({{.*}}).y, int(({{.*}}).z), (([[GX2]])), (([[GY2]])))
    outF4[3] = t2DArr.SampleGrad(samp, float3(0.5, 0.5, 1.0), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA: [[GX3:_S[0-9]+]] = make_float3{{ *}}(0.25f, 0.0f, 0.0f);
    // CUDA: [[GY3:_S[0-9]+]] = make_float3{{ *}}(0.0f, 0.125f, 0.0f);
    // CUDA: tex3DGrad<float4 >({{.*}}t3D_0), ({{.*}}).x, ({{.*}}).y, ({{.*}}).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[4] = t3D.SampleGrad(samp, float3(0.5, 0.5, 0.5), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA: texCubemapGrad<float4 >({{.*}}tCube_0), ({{.*}}).x, ({{.*}}).y, ({{.*}}).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[5] = tCube.SampleGrad(samp, float3(1.0, 0.0, 0.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA: texCubemapLayeredGrad<float4 >({{.*}}tCubeArr_0), ({{.*}}).x, ({{.*}}).y, ({{.*}}).z, int(({{.*}}).w), make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[6] = tCubeArr.SampleGrad(samp, float4(1.0, 0.0, 0.0, 1.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // A 3-component texel type fetches four components and reshapes to float3; the
    // 3-component template argument is never instantiated. Each negative check
    // precedes its positive so the exclusion window ends at the correct fetch.
    // CUDA-NOT: tex1DGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex1DGrad<float4>
    outF3[0] = t1DF3.SampleGrad(samp, 0.5, 0.25, 0.125);

    // CUDA-NOT: tex1DLayeredGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex1DLayeredGrad<float4>
    outF3[1] = t1DArrF3.SampleGrad(samp, float2(0.5, 1.0), 0.25, 0.125);

    // CUDA-NOT: tex2DGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex2DGrad<float4>
    outF3[2] = t2DF3.SampleGrad(samp, float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: tex2DLayeredGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex2DLayeredGrad<float4>
    outF3[3] = t2DArrF3.SampleGrad(samp, float3(0.5, 0.5, 1.0), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: tex3DGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex3DGrad<float4>
    outF3[4] = t3DF3.SampleGrad(samp, float3(0.5, 0.5, 0.5), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: texCubemapGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(texCubemapGrad<float4>
    outF3[5] = tCubeF3.SampleGrad(samp, float3(1.0, 0.0, 0.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: texCubemapLayeredGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(texCubemapLayeredGrad<float4>
    outF3[6] = tCubeArrF3.SampleGrad(samp, float4(1.0, 0.0, 0.0, 1.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: tex2DGrad<int3
    // CUDA: _slang_vector_reshape<int, 3, int, 4>(tex2DGrad<int4>
    outI3[0] = t2DI3.SampleGrad(samp, float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: tex2DGrad<uint3
    // CUDA: _slang_vector_reshape<uint, 3, uint, 4>(tex2DGrad<uint4>
    outU3[0] = t2DU3.SampleGrad(samp, float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: _slang_vector_reshape<float, 1
    // CUDA: tex2DGrad<float>
    outF1[0] = t2DF1.SampleGrad(samp, float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: _slang_vector_reshape<float, 2
    // CUDA: tex2DGrad<float2 >
    outF2[0] = t2DF2.SampleGrad(samp, float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // Combined texture-sampler form, every shape, with the same argument lists.
    // CUDA: tex1DGrad<float4 >({{.*}}c1D_0), ((0.5f)), ((0.25f)), ((0.125f)))
    outF4[7] = c1D.SampleGrad(0.5, 0.25, 0.125);

    // CUDA: tex1DLayeredGrad<float4 >({{.*}}c1DArr_0), ({{.*}}).x, int(({{.*}}).y), ((0.25f)), ((0.125f)))
    outF4[8] = c1DArr.SampleGrad(float2(0.5, 1.0), 0.25, 0.125);

    // CUDA: tex2DGrad<float4 >({{.*}}c2D_0), ({{.*}}).x, ({{.*}}).y, (([[GX2]])), (([[GY2]])))
    outF4[9] = c2D.SampleGrad(float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA: tex2DLayeredGrad<float4 >({{.*}}c2DArr_0), ({{.*}}).x, ({{.*}}).y, int(({{.*}}).z), (([[GX2]])), (([[GY2]])))
    outF4[10] = c2DArr.SampleGrad(float3(0.5, 0.5, 1.0), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA: tex3DGrad<float4 >({{.*}}c3D_0), ({{.*}}).x, ({{.*}}).y, ({{.*}}).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[11] = c3D.SampleGrad(float3(0.5, 0.5, 0.5), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA: texCubemapGrad<float4 >({{.*}}cCube_0), ({{.*}}).x, ({{.*}}).y, ({{.*}}).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[12] = cCube.SampleGrad(float3(1.0, 0.0, 0.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA: texCubemapLayeredGrad<float4 >({{.*}}cCubeArr_0), ({{.*}}).x, ({{.*}}).y, ({{.*}}).z, int(({{.*}}).w), make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[13] = cCubeArr.SampleGrad(float4(1.0, 0.0, 0.0, 1.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: tex3DGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex3DGrad<float4>
    outF3[7] = c3DF3.SampleGrad(float3(0.5, 0.5, 0.5), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));
}

// The PTX target compiles through NVRTC, which accepts every argument list above and
// emits an explicit-gradient texture instruction for each shape and element type.
// PTX-DAG: tex.grad.1d.v4.f32.f32
// PTX-DAG: tex.grad.a1d.v4.f32.f32
// PTX-DAG: tex.grad.2d.v4.f32.f32
// PTX-DAG: tex.grad.a2d.v4.f32.f32
// PTX-DAG: tex.grad.3d.v4.f32.f32
// PTX-DAG: tex.grad.cube.v4.f32.f32
// PTX-DAG: tex.grad.acube.v4.f32.f32
// PTX-DAG: tex.grad.2d.v4.s32.f32
// PTX-DAG: tex.grad.2d.v4.u32.f32
