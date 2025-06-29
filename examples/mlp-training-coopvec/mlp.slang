module mlp;

import common;

__include mlvec;

// We use Float16 for the CoopVec component type since it is more widely supported.
//
static const CoopVecComponentType kComponentType = CoopVecComponentType.Float16;

public struct FeedForwardLayer<int InputSize, int OutputSize>
{
    internal void* weights;
    internal void* weightsGrad;
    internal void* biases;
    internal void* biasesGrad;

    public MLVec<OutputSize> eval(MLVec<InputSize> input)
    {
        // Compute mul(weights, inputVec) + biases.
        // `weights` is treated as an OutputSize(row) x InputSize(col) matrix.
        var output = coopVecMatMulAdd<NFloat, OutputSize>(
            input.data, kComponentType, // input and format
            weights, kComponentType, // weights and format
            biases, kComponentType, // biases and format
            CoopVecMatrixLayout.RowMajor, // matrix layout
            false, // transpose matrix? must be `false` since we specified RowMajor.
            InputSize * sizeof(NFloat)); // matrix stride
        output = max(output, output * 0.001h); // Leaky ReLU activation
        return {output};
    }

    [BackwardDerivativeOf(eval)]
    public void evalBwd(
        inout DifferentialPair<MLVec<InputSize>> input,
        MLVec<OutputSize> resultGrad)
    {
        let fwd = eval(input.p);

        // Back-prop resultGrad through activation.
        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
        {
            if (fwd.data[i] < 0.0)
                resultGrad.data[i] *= 0.01h;
        }

        // Back-prop gradients to the weights matrix.
        coopVecOuterProductAccumulate(
            resultGrad.data,
            input.p.data,
            weightsGrad,
            0, // matrixStride, ignored since layout is TrainingOptimal
            CoopVecMatrixLayout.TrainingOptimal, // matrix layout, must be TrainingOptimal.
            kComponentType);
        
        // Back-prop gradients to the biases vector.
        coopVecReduceSumAccumulate(resultGrad.data, (void*)biasesGrad);

        // Back-prop gradients to the input vector by computing
        // mul(transpose(weights), resultGrad).
        // By specifying the matrix layout as ColumnMajor, we can
        // achieve the effect of transposing the weights matrix.
        let dInput = coopVecMatMul<NFloat, InputSize>(
            resultGrad.data, kComponentType,
            weights, kComponentType,
            CoopVecMatrixLayout.ColumnMajor,
            false,  // transpose, must be `false` since we specified ColumnMajor.
            InputSize * sizeof(NFloat));

        input = {input.p, {dInput}};
    }
}
