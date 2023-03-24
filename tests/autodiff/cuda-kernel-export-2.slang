//DISABLE_TEST:SIMPLE: -target cuda -line-directive-mode none

// Verify that we can output a cuda device function with [CudaDeviceExport].
// Disabled until we have FileCheck.


//////////////////////////////////////////////////////////////////////////
// Lambda GGX
//////////////////////////////////////////////////////////////////////////

[CudaDeviceExport]
[BackwardDifferentiable]
float lambdaGGX(const float alphaSqr, const float cosTheta)
{
    const float SPECULAR_EPSILON = 1e-4f;
    float _cosTheta = clamp(cosTheta, SPECULAR_EPSILON, 1.0f - SPECULAR_EPSILON);
    float cosThetaSqr = _cosTheta * _cosTheta;
    float tanThetaSqr = (1.0 - cosThetaSqr) / cosThetaSqr;
    return 0.5f * (sqrt(1.0f + alphaSqr * tanThetaSqr) - 1.0f);
}

[CudaDeviceExport]
void lambdaGGX_bwd(inout DifferentialPair<float> alphaSqr, inout DifferentialPair<float> cosTheta, const float d_out)
{
    __bwd_diff(lambdaGGX)(alphaSqr, cosTheta, d_out);
}

//////////////////////////////////////////////////////////////////////////
// Masking Smith
//////////////////////////////////////////////////////////////////////////

[CudaDeviceExport]
[BackwardDifferentiable]
float maskingSmithGGXCorrelated(const float alphaSqr, const float cosThetaI, const float cosThetaO)
{
    float lambdaI = lambdaGGX(alphaSqr, cosThetaI);
    float lambdaO = lambdaGGX(alphaSqr, cosThetaO);
    return 1.0f / (1.0f + lambdaI + lambdaO);
}

[CudaDeviceExport]
void maskingSmithGGXCorrelated_bwd(inout DifferentialPair<float> alphaSqr,
                                   inout DifferentialPair<float> cosThetaI,
                                   inout DifferentialPair<float> cosThetaO,
                                   const float d_out)
{
    __bwd_diff(maskingSmithGGXCorrelated)(alphaSqr, cosThetaI, cosThetaO, d_out);
}
