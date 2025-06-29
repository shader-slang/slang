module mlp_sw;

import common;

__include mlvec_sw;

public struct FeedForwardLayer<int InputSize, int OutputSize>
{
    public NFloat* weights;
    public NFloat* weightsGrad;
    public NFloat* biases;
    public NFloat* biasesGrad;

    [BackwardDerivative(evalBwd)]
    public MLVec<OutputSize> eval(MLVec<InputSize> input)
    {
        var output = matMulAdd<OutputSize>(
            input,
            weights,
            biases);
        // ReLU activation
        for (int i = 0; i < OutputSize; i++)
            if (output.data[i] < 0.0)
                output.data[i] *= 0.001h;
        return output; 
    }

    public void evalBwd(
        inout DifferentialPair<MLVec<InputSize>> input,
        MLVec<OutputSize> resultGrad)
    {
        let fwd = eval(input.p);

        // Back-prop resultGrad through activation.
        for (int i = 0; i < OutputSize; i++)
        {
            if (fwd.data[i] < 0.0)
                resultGrad.data[i] *= 0.01h;
        }

        // Back-prop gradients to the weights matrix.
        outerProductAccumulate(
            resultGrad,
            input.p,
            weightsGrad);

        // Back-prop gradients to the biases vector.
        for (int i = 0; i < OutputSize; i++)
        {
            NFloat originalValue;
            InterlockedAddF16Emulated(biasesGrad + i, resultGrad.data[i], originalValue);
        }

        // Back-prop gradients to the input vector.
        let dInput = matMulTransposed<InputSize>(resultGrad, weights);

        input = {input.p, dInput};
    }
}
