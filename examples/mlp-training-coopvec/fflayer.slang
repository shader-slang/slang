implementing mlp;

// We use Float16 for the CoopVec component type since it is more widely supported.
//
static const CoopVecComponentType kComponentType = CoopVecComponentType.Float16;

public struct FeedForwardLayer<int InputSize, int OutputSize>
{
    internal void* weights;
    internal void* weightsGrad;
    internal void* biases;
    internal void* biasesGrad;

    // Matrix operations depend on layout metadata and are not natively differentiable, so keep
    // coopVecMatMulAdd behind a wrapper with a custom reverse derivative.
    internal CoopVec<NFloat, OutputSize> matMulAdd(CoopVec<NFloat, InputSize> input)
    {
        // Compute mul(weights, inputVec) + biases.
        // `weights` is treated as an OutputSize(row) x InputSize(col) matrix.
        return coopVecMatMulAdd<NFloat, OutputSize>(
            input, kComponentType, // input and format
            weights, kComponentType, // weights and format
            biases, kComponentType, // biases and format
            CoopVecMatrixLayout.RowMajor, // matrix layout
            false, // transpose matrix? must be `false` since we specified RowMajor.
            InputSize * sizeof(NFloat)); // matrix stride
    }

    [BackwardDerivativeOf(matMulAdd)]
    internal void matMulAddBwd(
        inout DifferentialPair<CoopVec<NFloat, InputSize>> input,
        CoopVec<NFloat, OutputSize> resultGrad)
    {
        // Back-prop gradients to the weights matrix.
        coopVecOuterProductAccumulate(
            resultGrad,
            input.p,
            weightsGrad,
            0, // matrixStride, ignored since layout is TrainingOptimal
            CoopVecMatrixLayout.TrainingOptimal, // matrix layout, must be TrainingOptimal.
            kComponentType);

        // Back-prop gradients to the biases vector.
        coopVecReduceSumAccumulate(resultGrad, (void*)biasesGrad);

        // Back-prop gradients to the input vector by computing
        // mul(transpose(weights), resultGrad).
        // By specifying the matrix layout as ColumnMajor, we can
        // achieve the effect of transposing the weights matrix.
        let dInput = coopVecMatMul<NFloat, InputSize>(
            resultGrad, kComponentType,
            weights, kComponentType,
            CoopVecMatrixLayout.ColumnMajor,
            false,  // transpose, must be `false` since we specified ColumnMajor.
            InputSize * sizeof(NFloat));

        input = diffPair(input.p, dInput);
    }

    [Differentiable]
    public CoopVec<NFloat, OutputSize> eval(CoopVec<NFloat, InputSize> input)
    {
        let output = matMulAdd(input);
        let negativeSlope = CoopVec<NFloat, OutputSize>(0.001h);
        return max(output, output * negativeSlope); // Leaky ReLU activation
    }
}
