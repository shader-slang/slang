//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=CPP): -target cpp -whole-program -line-directive-mode none -validate-ir

// Keep Payload separate from the live scalar in internal propagation signatures.
// CUDA can then discard the unused receiver independently, while the exported
// ABI and any remaining context construction retain their original types.
// CUDA: static __device__ void s_bwdProp_Payload_setup_{{[0-9]+}}(float
// CUDA-SAME: , float * {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}})
// CUDA: static __device__ void s_bwdProp_evaluate_{{[0-9]+}}(Payload_{{[0-9]+}} *
// CUDA-SAME: , float {{[[:alnum:]_]+}}, float * {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}})
// CUDA: __device__ void backward(Payload_{{[0-9]+}} payload_{{[0-9]+}},
// The same input keeps its context parameter on a non-CUDA target.
// CPP: static void s_bwdProp_evaluate_{{[0-9]+}}(s_bwdCallableCtx_evaluate_{{[0-9]+}} *

struct Payload
{
    float unusedData[256];

    [Differentiable]
    [PreferRecompute]
    float setup(float x)
    {
        return x * x;
    }
}

[Differentiable]
float evaluate(Payload payload, float x)
{
    float a = payload.setup(x);
    return sin(a) * cos(x);
}

[CudaDeviceExport]
void backward(Payload payload, inout DifferentialPair<float> x, float derivative)
{
    bwd_diff(evaluate)(payload, x, derivative);
}
