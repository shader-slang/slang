//TEST:SIMPLE(filecheck=VK_VTX):              -target spirv -stage vertex   -entry vertexMain
//TEST:SIMPLE(filecheck=VK_FRAG):             -target spirv -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=VK_COMPUTE_NODERIVS): -target spirv -stage compute  -entry computeMain
//TEST:SIMPLE(filecheck=VK_COMPUTE_DERIVS):   -target spirv -stage compute  -entry computeMain -capability SPV_KHR_compute_shader_derivatives -DCOMPUTE_DERIVS
//TEST:SIMPLE(filecheck=GLSL_VTX):  -target glsl  -stage vertex   -entry vertexMain
//TEST:SIMPLE(filecheck=GLSL_FRAG): -target glsl  -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=HLSL_VTX):  -target hlsl  -stage vertex   -entry vertexMain
//TEST:SIMPLE(filecheck=HLSL_FRAG): -target hlsl  -stage fragment -entry fragmentMain

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

Sampler1D<float> uniform_sampler1D;

int returnSupportDerivatives()
{
    return int(__supportImplicitDerivatives());
}

[ForceInline]
int sampleWithImplicitOrExplicitLod()
{
    if (__supportImplicitDerivatives())
    {
        return int(100 * uniform_sampler1D.Sample(float(0)));
    }
    else
    {
        return int(100 * uniform_sampler1D.SampleLevelZero(float(0)));
    }
}

[shader("vertex")]
void vertexMain()
{
    // VK_VTX-NOT: OpImageSampleImplicitLod
    // VK_VTX:     OpImageSampleExplicitLod
    // VK_VTX-NOT: OpImageSampleImplicitLod

    // GLSL_VTX-NOT: textureLod(
    // GLSL_VTX:     texture(
    // GLSL_VTX-NOT: textureLod(

    // HLSL_VTX-NOT: Sample(
    // HLSL_VTX: SampleLevel(
    // HLSL_VTX-NOT: Sample(

    static_assert(!__supportImplicitDerivatives(), "Vertex shaders don't support implicit derivatives");
    outputBuffer[0] = returnSupportDerivatives();
    outputBuffer[1] = sampleWithImplicitOrExplicitLod();
}

[shader("fragment")]
void fragmentMain()
{
    // VK_FRAG-NOT: OpImageSampleExplicitLod
    // VK_FRAG:     OpImageSampleImplicitLod
    // VK_FRAG-NOT: OpImageSampleExplicitLod

    // GLSL_FRAG-NOT: textureLod(
    // GLSL_FRAG:     texture(
    // GLSL_FRAG-NOT: textureLod(

    // HLSL_FRAG-NOT: SampleLevel(
    // HLSL_FRAG:     Sample(
    // HLSL_FRAG-NOT: SampleLevel(

    static_assert(__supportImplicitDerivatives(), "Fragment shaders support implicit derivatives");
    outputBuffer[0] = returnSupportDerivatives();
    outputBuffer[1] = sampleWithImplicitOrExplicitLod();
}

[shader("compute")]
[numthreads(2, 2, 1)]
void computeMain()
{
    outputBuffer[0] = returnSupportDerivatives();

    // VK_COMPUTE_DERIVS-NOT: OpImageSampleExplicitLod
    // VK_COMPUTE_DERIVS:     OpImageSampleImplicitLod
    // VK_COMPUTE_DERIVS-NOT: OpImageSampleExplicitLod

    // VK_COMPUTE_NODERIVS-NOT: OpImageSampleImplicitLod
    // VK_COMPUTE_NODERIVS:     OpImageSampleExplicitLod
    // VK_COMPUTE_NODERIVS-NOT: OpImageSampleImplicitLod

#if defined(COMPUTE_DERIVS)
    static_assert(__supportImplicitDerivatives(), "Check that implicit derivatives are currently enabled in compute");
#else
    static_assert(!__supportImplicitDerivatives(), "Check that implicit derivatives are not currently enabled in compute");
#endif

    outputBuffer[0] = returnSupportDerivatives();
    outputBuffer[1] = sampleWithImplicitOrExplicitLod();
}
