// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3 -Xslang -warnings-disable -Xslang 31227 -Xslang -Itests/cuda
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -warnings-disable -Xslang 31227 -Xslang -Itests/cuda
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -warnings-disable -Xslang 31227 -Xslang -Itests/cuda

// The unchanged imported module uses constexpr variables treated as const (warning31227).
// Suppress only that known warning. This fixture qualifies the runtime dielectric policy;
// the registered tiled-brass graph remains reflection-only.
import complex.tiled_brass_material;

// Runtime material parameters are (alpha.x, alpha.y, eta, unused), with unequal roughness axes.
//TEST_INPUT:ubuffer(data=[0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0 0.5 0.25 2.0 0.0],stride=16):name=materialParameters
StructuredBuffer<float4> materialParameters;
// First four cases preserve the normal probe: both R/T, then R-only and T-only.
// Cases4-7 disable the requested direction or both lobes. Cases8-11 are both-mode
// x/y reflection/transmission witnesses; normalizing (3,0,+/-4)/(0,3,+/-4) is intentional.
//TEST_INPUT:ubuffer(data=[0.0 0.0 1.0 0.0 0.0 0.0 -1.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 -1.0 0.0 0.0 0.0 -1.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 -1.0 0.0 3.0 0.0 4.0 0.0 3.0 0.0 -4.0 0.0 0.0 3.0 4.0 0.0 0.0 3.0 -4.0 0.0],stride=16):name=outgoingDirections
StructuredBuffer<float4> outgoingDirections;
//TEST_INPUT:ubuffer(data=[0.25 0.5 0.75 0.0 0.25 0.5 0.75 0.0 0.25 0.5 0.75 0.0 0.0 0.0 0.0 0.0 0.25 0.5 0.75 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.25 0.5 0.75 0.0 0.25 0.5 0.75 0.0 0.25 0.5 0.75 0.0 0.25 0.5 0.75 0.0],stride=16):name=reflectionTints
StructuredBuffer<float4> reflectionTints;
//TEST_INPUT:ubuffer(data=[0.75 0.25 0.5 0.0 0.75 0.25 0.5 0.0 0.0 0.0 0.0 0.0 0.75 0.25 0.5 0.0 0.0 0.0 0.0 0.0 0.75 0.25 0.5 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.75 0.25 0.5 0.0 0.75 0.25 0.5 0.0 0.75 0.25 0.5 0.0 0.75 0.25 0.5 0.0],stride=16):name=transmissionTints
StructuredBuffer<float4> transmissionTints;

// Independent normal closed forms use F=1/9, D=8/pi, J=4, and bounded density factor17/16.
// Off-axis rows use a separate double geometric oracle. Values and PDFs are checked separately:
// Fresnel branch probability cancels in evaluated value but remains in the PDF.
//TEST_INPUT:ubuffer(data=[0.017683882266283035 0.03536776453256607 0.053051646798849106 0.07515650242567062 6.7906107902526855 2.2635369300842285 4.527073860168457 9.62003231048584 0.017683882266283035 0.03536776453256607 0.053051646798849106 0.6764085292816162 6.7906107902526855 2.2635369300842285 4.527073860168457 10.82253646850586 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.010141370818018913 0.020282741636037827 0.03042411431670189 0.04456626623868942 0.11646199971437454 0.03882066532969475 0.0776413306593895 0.17059744894504547 0.0028109706472605467 0.005621941294521093 0.008432912640273571 0.012050718069076538 0.008165979757905006 0.002721993485465646 0.005443986970931292 0.011669269762933254],stride=16):name=expected
StructuredBuffer<float4> expected;

//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=results
RWStructuredBuffer<uint> results;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint completed = 0;
    for (uint index = 0; index < 12; ++index)
    {
        float4 parameters = materialParameters[index];
        float3 wo = normalize(outgoingDirections[index].xyz);
        mtlx::MxDielectricNoCompensationBSDF bsdf = mtlx::MxDielectricNoCompensationBSDF(
            parameters.xy, mtlx::MxDielectricFresnel(parameters.z, false),
            reflectionTints[index].xyz, transmissionTints[index].xyz,
            float3(0.0), false, false);
        render::EvalPdfFallbackSampleGenerator sg = render::EvalPdfFallbackSampleGenerator(0u);
        render::BSDFEval evaluated = bsdf.eval_with_pdf(
            float3(0.0, 0.0, 1.0), wo, sg, render::BSDFContext(1.0, parameters.z));
        float4 actual = float4(evaluated.value, evaluated.pdf);
        float4 wanted = expected[index];
        uint errors = 0;
        for (uint channel = 0; channel < 4; ++channel)
        {
            // The pre-observation budget includes Float32 normalization and arithmetic.
            // Disabled cases require exact numerical zero, not a tolerance-sized residual.
            float budget = 0.000001 + 0.00001 * abs(wanted[channel]);
            if (!isfinite(actual[channel]) ||
                (wanted[channel] == 0.0 ? actual[channel] != 0.0 :
                    abs(actual[channel] - wanted[channel]) > budget))
                errors |= 1u << channel;
        }
        results[index] = errors;
        ++completed;
    }
    results[12] = completed;
    // Word13 remains an untouched guard.
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 12{{$}}
// CHECK-NEXT: 3735928559{{$}}
