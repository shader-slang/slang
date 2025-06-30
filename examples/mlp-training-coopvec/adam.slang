module adam;

import mlp;
import common;

public struct AdamState
{
    internal NFloat mean;
    internal NFloat variance;
    internal int iteration;
}

public struct AdamOptimizer
{
    // Adam parameters
    public static const NFloat beta1 = 0.9h;
    public static const NFloat beta2 = 0.999h;
    public static const NFloat epsilon = 1e-7h;
    public static const NFloat learningRate = 0.01h;

    public static void step(inout AdamState state, inout NFloat param, inout NFloat grad)
    {
        state.iteration++;
        if (isinf(grad))
        {
            if (grad > 0)
                grad = 10000.0h;
            else
                grad = -10000.0h;
        }
        state.mean = beta1 * state.mean + (NFloat(1.f) - beta1) * grad;
        state.variance = beta2 * state.variance + (NFloat(1.f) - beta2) * grad * grad;
        NFloat meanHat = state.mean / (NFloat(1.f) - pow(beta1, NFloat(state.iteration)));
        NFloat varianceHat = state.variance / (NFloat(1.f) - pow(beta2, NFloat(state.iteration)));
        param -= learningRate * meanHat / (sqrt(max(NFloat(0.f), varianceHat) + epsilon));
        grad = NFloat(0.f);
    }
}
