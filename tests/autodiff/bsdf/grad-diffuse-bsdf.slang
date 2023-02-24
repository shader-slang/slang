//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

__exported import DiffuseBSDF;

struct SurfaceData
{
    ShadingData sd;
    float3 wo;
};

float3 getAlbedo(bool useRef)
{
    float3 res;
    if (useRef) res = float3(0.9f, 0.6f, 0.2f);
    else res = float3(0.5, 0.5, 0.5);
    return res;
}

[BackwardDerivativeOf(getAlbedo)]
void __bwd_d_getAlbedo(bool useRef, float3.Differential dOut)
{
    if (!useRef)
    {
        outputBuffer[0] += dOut[0];
        outputBuffer[1] += dOut[1];
        outputBuffer[2] += dOut[2];
    }
}

[BackwardDifferentiable]
float3 evalBSDF(bool useRef, SurfaceData data)
{
    float3 baseColor = getAlbedo(useRef);
    DiffuseMaterialInstance mi = { data.sd, { baseColor } };
    float3 res = mi.eval(data.sd, data.wo);
    return res;
}

[BackwardDifferentiable]
float objectiveFunction(DiffuseBSDF curBSDF, no_diff DiffuseBSDF refBSDF)
{
    float3 wi = no_diff normalize(float3(0.3f, 0.2f, 0.8f));
    float3 wo = no_diff normalize(float3(-0.1f, -0.3f, 0.9f));

    float3 fRef = refBSDF.eval(wi, wo);
    float3 fCur = curBSDF.eval(wi, wo);
    return L2Loss(fCur, fRef);
}

[BackwardDifferentiable]
float L2Loss(float3 x, float3 xRef)
{
    float3 diff = x - xRef;
    return dot(diff, diff);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    ShadingData sd;
    sd.N = float3(0, 0, 1);
    sd.T = float3(1, 0, 0);
    sd.B = float3(0, 1, 0);
    sd.V = normalize(float3(0.3, 0.2, 0.8));

    float3 wo = normalize(float3(-0.1f, -0.3f, 0.9f));

    SurfaceData data = { sd, wo };
    __bwd_diff(evalBSDF)(false, data, float3(1.0));

    /*
    DiffuseBSDF refBSDF = { float3(0.9f, 0.6f, 0.2f) };
    DiffuseBSDF curBSDF = { float3(0.5f, 0.5f, 0.5f) };

    DifferentialPair<DiffuseBSDF> dp_curBSDF = DifferentialPair<DiffuseBSDF>(
        { float3(0.5f, 0.5f, 0.5f) }, { float3(0.f) });

    __bwd_diff(objectiveFunction)(dp_curBSDF, refBSDF, 1.f);
    

    outputBuffer[0] = dp_curBSDF.d.albedo[0];
    outputBuffer[1] = dp_curBSDF.d.albedo[1];
    outputBuffer[2] = dp_curBSDF.d.albedo[2];
    */
}
