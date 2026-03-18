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

public void adam_step(inout float primal, inout float grad,
                      inout float mean, inout float variance,
                      float lr, int iter)
{
    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    mean = lerp(grad, mean, BETA1);
    variance = lerp(grad * grad, variance, BETA2);
    float m_hat = mean / (1.0f - pow(BETA1, iter));
    float v_hat = variance / (1.0f - pow(BETA2, iter));
    primal -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    grad = 0.0f;
}
