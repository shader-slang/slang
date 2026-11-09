//TEST:SIMPLE(filecheck=CUDA):-target cuda -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Regression coverage for https://github.com/shader-slang/slang/issues/11160.
// Differentiate a covariance projection that builds its Jacobian using fwd_diff and out.
// CUDA-DAG: __device__ void projectWithCovariance(
// CUDA-DAG: __device__ void projectWithCovarianceVjp(

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
void project(float3 xyz, out float2 uv)
{
    uv = float2(xyz.x / xyz.z, xyz.y / xyz.z);
}

[Differentiable]
float2x3 projectionJacobian(float3 xyz)
{
    float2x3 jacobian;
    [ForceUnroll]
    for (int i = 0; i < 3; ++i)
    {
        var dpXyz = diffPair(xyz, float3(float(i == 0), float(i == 1), float(i == 2)));
        DifferentialPair<float2> dpUv;
        fwd_diff(project)(dpXyz, dpUv);
        jacobian[0][i] = dpUv.d.x;
        jacobian[1][i] = dpUv.d.y;
    }
    return jacobian;
}

[CudaDeviceExport]
[Differentiable]
void projectWithCovariance(
    float3 xyz,
    float3x3 xyzCovariance,
    out float2 uv,
    out float2x2 uvCovariance)
{
    project(xyz, uv);
    float2x3 jacobian = projectionJacobian(xyz);
    uvCovariance = mul(mul(jacobian, xyzCovariance), transpose(jacobian));
}

[CudaDeviceExport]
void projectWithCovarianceVjp(
    float3 xyz,
    float3x3 xyzCovariance,
    float2 uvGradient,
    float2x2 uvCovarianceGradient,
    out float3 xyzGradient,
    out float3x3 xyzCovarianceGradient)
{
    var dpXyz = diffPair(xyz);
    var dpXyzCovariance = diffPair(xyzCovariance);
    bwd_diff(projectWithCovariance)(dpXyz, dpXyzCovariance, uvGradient, uvCovarianceGradient);
    xyzGradient = dpXyz.d;
    xyzCovarianceGradient = dpXyzCovariance.d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float3 xyzGradient;
    float3x3 xyzCovarianceGradient;
    projectWithCovarianceVjp(
        float3(2, 3, 4),
        float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1),
        float2(1, 2),
        float2x2(1, 0, 0, 1),
        xyzGradient,
        xyzCovarianceGradient);

    // With identity input covariance and these output seeds, the objective is
    // x/z + 2*y/z + 2/z^2 + (x^2+y^2)/z^4.
    // CHECK: 0.265625
    outputBuffer[0] = xyzGradient.x;
    // CHECK: 0.523438
    outputBuffer[1] = xyzGradient.y;
    // CHECK: -0.613281
    outputBuffer[2] = xyzGradient.z;

    // The covariance gradient is transpose(jacobian) * jacobian.
    // CHECK: 0.062500
    // CHECK: 0.000000
    // CHECK: -0.031250
    // CHECK: 0.000000
    // CHECK: 0.062500
    // CHECK: -0.046875
    // CHECK: -0.031250
    // CHECK: -0.046875
    // CHECK: 0.050781
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            outputBuffer[3 + i * 3 + j] = xyzCovarianceGradient[i][j];
}
