//DISABLE_TEST:SIMPLE: -target cuda -line-directive-mode none

// Verify that we can output a cuda device function with [CudaDeviceExport].
// Disabled until we have FileCheck.

struct MixedType : IDifferentiable
{
    no_diff float noDiffField;
    float field;
}

[BackwardDifferentiable]
float f1(MixedType m)
{
    return 2.0 * m.field;
}

[BackwardDifferentiable]
float f(MixedType m)
{
    MixedType m1 = { m.noDiffField, m.field };
    return f1(m1);
}

[CudaDeviceExport]
void diffF(inout DifferentialPair<MixedType> m, float dout)
{
    __bwd_diff(f)(m, dout);
}

[CudaKernel]
void myKernel(float* inValues, float* outValues)
{
    outValues[0] = sin(inValues[0]);
}

[CudaHost]
public __extern_cpp void runCompute(float *inValues, float *outValues, uint3 dispathcSize)
{
    __dispatch_kernel(myKernel, uint3(128, 1, 1), dispathcSize)(inValues, outValues);
}