// The combined texture-sampler SampleGrad lowers to the CUDA `tex*Grad<T>` runtime built-ins for
// every texture shape (shader-slang/slang#13556). The built-ins take float4 gradients for 3D and
// cube shapes, so float3 gradients widen; 3-component texel types fetch four components and reshape.
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

Sampler1D<float4> c1D;
Sampler1DArray<float4> c1DArr;
Sampler2D<float4> c2D;
Sampler2DArray<float4> c2DArr;
Sampler3D<float4> c3D;
SamplerCube<float4> cCube;
SamplerCubeArray<float4> cCubeArr;

Sampler1D<float3> c1DF3;
Sampler1DArray<float3> c1DArrF3;
Sampler2D<float3> c2DF3;
Sampler2DArray<float3> c2DArrF3;
Sampler3D<float3> c3DF3;
SamplerCube<float3> cCubeF3;
SamplerCubeArray<float3> cCubeArrF3;
SamplerCubeArray<int3> cCubeArrI3;
SamplerCubeArray<uint3> cCubeArrU3;
Sampler2D<float> c2DF1;
Sampler2D<float2> c2DF2;

RWStructuredBuffer<float> outF1;
RWStructuredBuffer<float2> outF2;
RWStructuredBuffer<float3> outF3;
RWStructuredBuffer<float4> outF4;
RWStructuredBuffer<int3> outI3;
RWStructuredBuffer<uint3> outU3;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Each call pins its full argument list: the coordinates, then the int layer for array
    // shapes, then gradX before gradY. The emitter hoists the vector coordinate and gradient
    // constants into temporaries that later calls share, so the checks capture those names
    // from their declarations; 1D coordinates and gradients stay inline literals.
    // CUDA: tex1DGrad<float4 >({{.*}}c1D_0), ((0.5f)), ((0.25f)), ((0.125f)))
    outF4[0] = c1D.SampleGrad(0.5, 0.25, 0.125);

    // CUDA: [[L1:_S[0-9]+]] = make_float2{{ *}}(0.5f, 1.0f);
    // CUDA: tex1DLayeredGrad<float4 >({{.*}}c1DArr_0), (([[L1]])).x, int((([[L1]])).y), ((0.25f)), ((0.125f)))
    outF4[1] = c1DArr.SampleGrad(float2(0.5, 1.0), 0.25, 0.125);

    // CUDA: [[C2:_S[0-9]+]] = make_float2{{ *}}(0.5f, 0.5f);
    // CUDA: [[GX2:_S[0-9]+]] = make_float2{{ *}}(0.25f, 0.0f);
    // CUDA: [[GY2:_S[0-9]+]] = make_float2{{ *}}(0.0f, 0.125f);
    // CUDA: tex2DGrad<float4 >({{.*}}c2D_0), (([[C2]])).x, (([[C2]])).y, (([[GX2]])), (([[GY2]])))
    outF4[2] = c2D.SampleGrad(float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA: [[L2:_S[0-9]+]] = make_float3{{ *}}(0.5f, 0.5f, 1.0f);
    // CUDA: tex2DLayeredGrad<float4 >({{.*}}c2DArr_0), (([[L2]])).x, (([[L2]])).y, int((([[L2]])).z), (([[GX2]])), (([[GY2]])))
    outF4[3] = c2DArr.SampleGrad(float3(0.5, 0.5, 1.0), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA: [[C3:_S[0-9]+]] = make_float3{{ *}}(0.5f, 0.5f, 0.5f);
    // CUDA: [[GX3:_S[0-9]+]] = make_float3{{ *}}(0.25f, 0.0f, 0.0f);
    // CUDA: [[GY3:_S[0-9]+]] = make_float3{{ *}}(0.0f, 0.125f, 0.0f);
    // CUDA: tex3DGrad<float4 >({{.*}}c3D_0), (([[C3]])).x, (([[C3]])).y, (([[C3]])).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[4] = c3D.SampleGrad(float3(0.5, 0.5, 0.5), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA: [[CC:_S[0-9]+]] = make_float3{{ *}}(1.0f, 0.0f, 0.0f);
    // CUDA: texCubemapGrad<float4 >({{.*}}cCube_0), (([[CC]])).x, (([[CC]])).y, (([[CC]])).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[5] = cCube.SampleGrad(float3(1.0, 0.0, 0.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA: [[LC:_S[0-9]+]] = make_float4{{ *}}(1.0f, 0.0f, 0.0f, 1.0f);
    // CUDA: texCubemapLayeredGrad<float4 >({{.*}}cCubeArr_0), (([[LC]])).x, (([[LC]])).y, (([[LC]])).z, int((([[LC]])).w), make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f))
    outF4[6] = cCubeArr.SampleGrad(float4(1.0, 0.0, 0.0, 1.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // A 3-component texel type fetches four components and reshapes to three; the
    // 3-component template argument is never instantiated. Each negative check
    // precedes its positive so the exclusion window ends at the correct fetch.
    // CUDA-NOT: tex1DGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex1DGrad<float4>({{.*}}c1DF3_0), ((0.5f)), ((0.25f)), ((0.125f))))
    outF3[0] = c1DF3.SampleGrad(0.5, 0.25, 0.125);

    // CUDA-NOT: tex1DLayeredGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex1DLayeredGrad<float4>({{.*}}c1DArrF3_0), (([[L1]])).x, int((([[L1]])).y), ((0.25f)), ((0.125f))))
    outF3[1] = c1DArrF3.SampleGrad(float2(0.5, 1.0), 0.25, 0.125);

    // CUDA-NOT: tex2DGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex2DGrad<float4>({{.*}}c2DF3_0), (([[C2]])).x, (([[C2]])).y, (([[GX2]])), (([[GY2]]))))
    outF3[2] = c2DF3.SampleGrad(float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: tex2DLayeredGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex2DLayeredGrad<float4>({{.*}}c2DArrF3_0), (([[L2]])).x, (([[L2]])).y, int((([[L2]])).z), (([[GX2]])), (([[GY2]]))))
    outF3[3] = c2DArrF3.SampleGrad(float3(0.5, 0.5, 1.0), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: tex3DGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(tex3DGrad<float4>({{.*}}c3DF3_0), (([[C3]])).x, (([[C3]])).y, (([[C3]])).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f)))
    outF3[4] = c3DF3.SampleGrad(float3(0.5, 0.5, 0.5), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: texCubemapGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(texCubemapGrad<float4>({{.*}}cCubeF3_0), (([[CC]])).x, (([[CC]])).y, (([[CC]])).z, make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f)))
    outF3[5] = cCubeF3.SampleGrad(float3(1.0, 0.0, 0.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: texCubemapLayeredGrad<float3
    // CUDA: _slang_vector_reshape<float, 3, float, 4>(texCubemapLayeredGrad<float4>({{.*}}cCubeArrF3_0), (([[LC]])).x, (([[LC]])).y, (([[LC]])).z, int((([[LC]])).w), make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f)))
    outF3[6] = cCubeArrF3.SampleGrad(float4(1.0, 0.0, 0.0, 1.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: texCubemapLayeredGrad<int3
    // CUDA: _slang_vector_reshape<int, 3, int, 4>(texCubemapLayeredGrad<int4>({{.*}}cCubeArrI3_0), (([[LC]])).x, (([[LC]])).y, (([[LC]])).z, int((([[LC]])).w), make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f)))
    outI3[0] = cCubeArrI3.SampleGrad(float4(1.0, 0.0, 0.0, 1.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: texCubemapLayeredGrad<uint3
    // CUDA: _slang_vector_reshape<uint, 3, uint, 4>(texCubemapLayeredGrad<uint4>({{.*}}cCubeArrU3_0), (([[LC]])).x, (([[LC]])).y, (([[LC]])).z, int((([[LC]])).w), make_float4((([[GX3]])).x, (([[GX3]])).y, (([[GX3]])).z, 0.0f), make_float4((([[GY3]])).x, (([[GY3]])).y, (([[GY3]])).z, 0.0f)))
    outU3[0] = cCubeArrU3.SampleGrad(float4(1.0, 0.0, 0.0, 1.0), float3(0.25, 0.0, 0.0), float3(0.0, 0.125, 0.0));

    // CUDA-NOT: _slang_vector_reshape<float, 1
    // CUDA: tex2DGrad<float>({{.*}}c2DF1_0), (([[C2]])).x, (([[C2]])).y, (([[GX2]])), (([[GY2]])))
    outF1[0] = c2DF1.SampleGrad(float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));

    // CUDA-NOT: _slang_vector_reshape<float, 2
    // CUDA: tex2DGrad<float2 >({{.*}}c2DF2_0), (([[C2]])).x, (([[C2]])).y, (([[GX2]])), (([[GY2]])))
    outF2[0] = c2DF2.SampleGrad(float2(0.5, 0.5), float2(0.25, 0.0), float2(0.0, 0.125));
}

// The PTX target compiles the emitted calls through NVRTC. FileCheck requires an
// explicit-gradient instruction for each of the seven float shapes, plus the signed and
// unsigned cube-array variants.
// PTX-DAG: tex.grad.1d.v4.f32.f32
// PTX-DAG: tex.grad.a1d.v4.f32.f32
// PTX-DAG: tex.grad.2d.v4.f32.f32
// PTX-DAG: tex.grad.a2d.v4.f32.f32
// PTX-DAG: tex.grad.3d.v4.f32.f32
// PTX-DAG: tex.grad.cube.v4.f32.f32
// PTX-DAG: tex.grad.acube.v4.f32.f32
// PTX-DAG: tex.grad.acube.v4.s32.f32
// PTX-DAG: tex.grad.acube.v4.u32.f32
