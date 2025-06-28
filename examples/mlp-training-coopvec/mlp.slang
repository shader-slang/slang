module mlp;

import common;

__include mlvec;

static const CoopVecComponentType kComponentType = CoopVecComponentType.Float16;

public struct FeedForwardLayer<int InputSize, int OutputSize>
{
    public NFloat* weights;
    public NFloat* weightsGrad;
    public NFloat* biases;
    public NFloat* biasesGrad;

    public MLVec<OutputSize> eval(MLVec<InputSize> input)
    {
        CoopVec<NFloat, InputSize> inputVec = input.data;
        var output = coopVecMatMulAdd<NFloat, OutputSize>(
            inputVec, kComponentType, // input and format
            (void*)weights, kComponentType, // weights and format
            (void*)biases, kComponentType, // biases and format
            CoopVecMatrixLayout.RowMajor, // matrix layout
            false, // is matrix transposed
            InputSize*sizeof(NFloat)); // matrix stride
        for (int i = 0; i < OutputSize; i++)
            if (output[i] < 0.0)
                output[i] *= 0.001h;
        return {output}; // ReLU activation
    }

    [BackwardDerivativeOf(eval)]
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

        CoopVec<NFloat, InputSize> inputV = input.p.data;
        CoopVec<NFloat, OutputSize> resultGradV = resultGrad.data;

        // Back-prop gradients to the weights matrix.
        coopVecOuterProductAccumulate(
            resultGradV,
            inputV,
            (void*)weightsGrad,
            0,
            CoopVecMatrixLayout.TrainingOptimal,
            kComponentType);
        
        // Back-prop gradients to the biases vector.
        coopVecReduceSumAccumulate(resultGradV, (void*)biasesGrad);

        // Back-prop gradients to the input vector.        
        let dInput = coopVecMatMul<NFloat, InputSize>(
            resultGradV, kComponentType,
            (void*)weights, kComponentType,
            CoopVecMatrixLayout.ColumnMajor,
            false,
            InputSize*sizeof(NFloat));

        input = {input.p, {dInput}};
    }
}
