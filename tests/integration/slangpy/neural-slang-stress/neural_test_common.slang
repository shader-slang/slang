// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Shared utilities for neural stress tests: LCG random number generator
// and Adam optimizer.

module neural_test_common;

public struct LCG
{
    public uint state;
    public __init(uint seed) { state = seed; }
    [mutating] public uint next_uint() { state = 1664525u * state + 1013904223u; return state; }
    [mutating] public float next_float() { return (next_uint() >> 8) * 0x1p-24; }
};

// Adam optimizer step for full-precision (fp32) training.
// Gradient is stored in-place and zeroed after the step.
public void adam_step(inout float primal, inout float grad,
                      inout float mean, inout float variance,
                      float lr, int iter, float loss_scale)
{
    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    float g = grad / loss_scale;
    mean = lerp(g, mean, BETA1);
    variance = lerp(g * g, variance, BETA2);
    float m_hat = mean / (1.0f - pow(BETA1, iter));
    float v_hat = variance / (1.0f - pow(BETA2, iter));
    primal -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    grad = 0.0f;
}

// Adam optimizer step for mixed-precision (fp16 forward, fp32 optimizer).
// Reads fp16 gradient (accumulated in half to match PointerAddress<half>),
// unscales, does Adam on fp32 master weights, copies back to fp16 param.
public void adam_step_mixed(inout half primal_half, inout half grad_half,
                            inout float primal_master,
                            inout float mean, inout float variance,
                            float lr, int iter, float loss_scale)
{
    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    float g = float(grad_half) / loss_scale;
    mean = lerp(g, mean, BETA1);
    variance = lerp(g * g, variance, BETA2);
    float m_hat = mean / (1.0f - pow(BETA1, iter));
    float v_hat = variance / (1.0f - pow(BETA2, iter));
    primal_master -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    primal_half = half(primal_master);
    grad_half = half(0.0f);
}
