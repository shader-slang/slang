// Textured MaterialX dielectric shading through the existing real triangle scene.
// Every ray-state, texture, stage and guard word is exact; BSDF lanes use the retained finite budget.
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -Itests/cuda -Xslang -warnings-disable -Xslang 31227,41016,41035 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -Itests/cuda -Xslang -warnings-disable -Xslang 31227,41016,41035 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -Itests/cuda -Xslang -warnings-disable -Xslang 31227,41016,41035 -Xslang -O3

import complex.tiled_brass_material;
// CUDA texture objects own sampler state; the source sampler is an intentional placeholder.
#pragma warning(disable: 41016)
#pragma warning(disable: 41035)

//TEST_INPUT: set scene = AccelerationStructure
RaytracingAccelerationStructure scene;
//TEST_INPUT: Texture2D(size=2, content=gradient):name materialTexture
Texture2D<float4> materialTexture;
//TEST_INPUT: ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

struct Payload { float4 value; };

void observeRay(uint x, uint stage)
{
    uint first = 25 * x;
    float3 origin = WorldRayOrigin();
    float3 direction = WorldRayDirection();
    output[first + 4] = asuint(origin.x);
    output[first + 5] = asuint(origin.y);
    output[first + 6] = asuint(origin.z);
    output[first + 7] = asuint(direction.x);
    output[first + 8] = asuint(direction.y);
    output[first + 9] = asuint(direction.z);
    output[first + 10] = asuint(RayTMin());
    output[first + 11] = asuint(RayTCurrent());
    output[first + 18] = stage;
    output[first + 19] = 0xc0010000u + x;
}

[shader("raygeneration")]
void raygenMain()
{
    uint x = DispatchRaysIndex().x;
    uint q = x >> 1;
    bool hit = (x & 1) == 0;
    float2 barycentrics = float2(0.125 + 0.25 * float(q & 1), 0.125 + 0.25 * float(q >> 1));
    RayDesc ray;
    // The existing RHI triangle's barycentric X is world Y and barycentric Y is world X.
    ray.Origin = hit ? float3(barycentrics.y, barycentrics.x, 1.0) :
        float3(2.0 + 0.25 * q, 3.0 + 0.5 * q, 1.5);
    ray.Direction = hit ? float3(0.0, 0.0, -1.0) : float3(0.125, -0.25, -1.0);
    ray.TMin = 0.125 + 0.03125 * q;
    ray.TMax = 8.0 + q;
    Payload payload;
    payload.value = float4(-11.0, -22.0, -33.0, -44.0);
    TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, payload);
    // Frozen normal-incidence closed forms, selected from the launch index rather than
    // the shader's sampled texture or returned hit attributes. No NVRTC values enter this oracle.
    float4 expected = hit ? float4(
        (q & 1) != 0 ? 0.05305164769729845 : 0.017683882565766147,
        (q >> 1) != 0 ? 0.05305164769729845 : 0.017683882565766147,
        0.017683882565766147, 0.07515650090450612) :
        float4(10.0 + q, 20.0 + 2 * q, 30.0 + 3 * q, 40.0 + 4 * q);
    for (uint channel = 0; channel < 4; ++channel)
    {
        float actual = payload.value[channel];
        output[25 * x + channel] = asuint(actual);
        bool correct = hit ? isfinite(actual) &&
            abs(actual - expected[channel]) <= 0.000001 + 0.00001 * abs(expected[channel]) :
            asuint(actual) == asuint(expected[channel]);
        output[25 * x + 20 + channel] = correct ? 0u : 1u;
    }
    // Per-ray word24 and the two outer words200/201 remain untouched.
}

[shader("miss")]
void missMain(inout Payload payload)
{
    uint x = DispatchRaysIndex().x;
    uint q = x >> 1;
    observeRay(x, 0x4d495353u);
    payload.value = float4(10.0 + q, 20.0 + 2 * q, 30.0 + 3 * q, 40.0 + 4 * q);
    // Miss leaves the barycentric and texture fields untouched as additional stage guards.
}

[shader("closesthit")]
void closestHitMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    uint x = DispatchRaysIndex().x;
    observeRay(x, 0x484954u);
    SamplerState sampler;
    float4 sampled = materialTexture.SampleLevel(sampler, 2.0 * attributes.barycentrics, 0.0);
    output[25 * x + 12] = asuint(attributes.barycentrics.x);
    output[25 * x + 13] = asuint(attributes.barycentrics.y);
    for (uint channel = 0; channel < 4; ++channel)
        output[25 * x + 14 + channel] = asuint(sampled[channel]);
    mtlx::MxDielectricNoCompensationBSDF bsdf = mtlx::MxDielectricNoCompensationBSDF(
        float2(0.5, 0.25), mtlx::MxDielectricFresnel(2.0, false),
        float3(0.25) + 0.5 * sampled.xyz, float3(0.75, 0.25, 0.5),
        float3(0.0), false, false);
    render::EvalPdfFallbackSampleGenerator sg = render::EvalPdfFallbackSampleGenerator(0u);
    render::BSDFEval evaluated = bsdf.eval_with_pdf(
        -WorldRayDirection(), float3(0.0, 0.0, 1.0), sg, render::BSDFContext(1.0, 2.0));
    payload.value = float4(evaluated.value, evaluated.pdf);
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1056964608{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}4737364{{$}}
// CHECK-NEXT: {{^}}3221291008{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1092616192{{$}}
// CHECK-NEXT: {{^}}1101004800{{$}}
// CHECK-NEXT: {{^}}1106247680{{$}}
// CHECK-NEXT: {{^}}1109393408{{$}}
// CHECK-NEXT: {{^}}1073741824{{$}}
// CHECK-NEXT: {{^}}1077936128{{$}}
// CHECK-NEXT: {{^}}1069547520{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}3196059648{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1090519040{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1296651091{{$}}
// CHECK-NEXT: {{^}}3221291009{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1042284544{{$}}
// CHECK-NEXT: {{^}}1056964608{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}4737364{{$}}
// CHECK-NEXT: {{^}}3221291010{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1093664768{{$}}
// CHECK-NEXT: {{^}}1102053376{{$}}
// CHECK-NEXT: {{^}}1107558400{{$}}
// CHECK-NEXT: {{^}}1110441984{{$}}
// CHECK-NEXT: {{^}}1074790400{{$}}
// CHECK-NEXT: {{^}}1080033280{{$}}
// CHECK-NEXT: {{^}}1069547520{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}3196059648{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1042284544{{$}}
// CHECK-NEXT: {{^}}1091567616{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1296651091{{$}}
// CHECK-NEXT: {{^}}3221291011{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1044381696{{$}}
// CHECK-NEXT: {{^}}1056964608{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}4737364{{$}}
// CHECK-NEXT: {{^}}3221291012{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1094713344{{$}}
// CHECK-NEXT: {{^}}1103101952{{$}}
// CHECK-NEXT: {{^}}1108344832{{$}}
// CHECK-NEXT: {{^}}1111490560{{$}}
// CHECK-NEXT: {{^}}1075838976{{$}}
// CHECK-NEXT: {{^}}1082130432{{$}}
// CHECK-NEXT: {{^}}1069547520{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}3196059648{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1044381696{{$}}
// CHECK-NEXT: {{^}}1092616192{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1296651091{{$}}
// CHECK-NEXT: {{^}}3221291013{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}{{[0-9]+}}{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1046478848{{$}}
// CHECK-NEXT: {{^}}1056964608{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}1052770304{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1065353216{{$}}
// CHECK-NEXT: {{^}}4737364{{$}}
// CHECK-NEXT: {{^}}3221291014{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1095761920{{$}}
// CHECK-NEXT: {{^}}1104150528{{$}}
// CHECK-NEXT: {{^}}1109131264{{$}}
// CHECK-NEXT: {{^}}1112539136{{$}}
// CHECK-NEXT: {{^}}1076887552{{$}}
// CHECK-NEXT: {{^}}1083179008{{$}}
// CHECK-NEXT: {{^}}1069547520{{$}}
// CHECK-NEXT: {{^}}1040187392{{$}}
// CHECK-NEXT: {{^}}3196059648{{$}}
// CHECK-NEXT: {{^}}3212836864{{$}}
// CHECK-NEXT: {{^}}1046478848{{$}}
// CHECK-NEXT: {{^}}1093664768{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}1296651091{{$}}
// CHECK-NEXT: {{^}}3221291015{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
